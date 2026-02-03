/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "checkers.h"
#include "game.h"
#include "game_interaction.h"
#include "game_checkers.h"
#include "game_display.h"
#include "game_net.h"
#include "menu.h"
#include "input.h"

struct move board_moves[64][MAX_MOVES];
int board_num_moves[64];

int sel_square;
int sel_piece_type;
int sel_piece_moves_len;
struct move *sel_piece_moves;
int sel_move_idx;
int player_turn = 0;

static void
set_sel_square(int i)
{
	sel_square = i;
	sel_piece_type =
		piece_occupying_square_belonging_to_player(i, player_turn);
	sel_piece_moves = board_moves[i];
	sel_piece_moves_len = board_num_moves[i];
}

static void
move_sel_square(int num)
{
	if (sel_square + num > 63)	set_sel_square(63);
	else if (sel_square + num < 0)	set_sel_square(0);
	else				set_sel_square(sel_square + num);
}

void
game_interaction_init(void)
{
	if (game_type == NETWORK)
		player_turn = game_net_player;
	else
		player_turn = 0;

	game_display_set_viewpoint(player_turn);
	if (player_turn == 0) {
		cur_mode = SELECT_PIECE;
		board_available_moves(board_moves, board_num_moves, player_turn, -1);
		set_sel_square(0);
	} else {
		cur_mode = WAIT_TURN;
	}
}

void
game_interaction_turn(void)
{
	cur_mode = SELECT_PIECE;
	board_available_moves(board_moves, board_num_moves, player_turn, -1);
	set_sel_square(player_turn == 0 ? 0 : 63);
}

static void
move_piece(void)
{
	int location = sel_piece_moves[sel_move_idx].location;
	int finished = perform_move(&sel_piece_moves[sel_move_idx],
				    player_turn);
	if (game_type == NETWORK)
		game_net_send_move(&sel_piece_moves[sel_move_idx]);
	sel_move_idx = 0;
	if (finished) {
		if (winner() != -1) {
			game_over();
			return;
		}

		if (game_type == NETWORK) {
			cur_mode = WAIT_TURN;
		} else {
			player_turn = !player_turn;
			board_available_moves(board_moves, board_num_moves,
					      player_turn, -1);
			game_start_anim_rotate();
			set_sel_square(player_turn == 0 ? 0 : 63);
		}
	} else {
		board_available_moves(board_moves, board_num_moves,
				      player_turn, location);
		set_sel_square(location);
	}
}

void
game_button_event(int button)
{
	if (button == INPUT_PAUSE) {
		menu.init();
		checkers_switch_state(&menu);
		return;
	}

	if (cur_mode == SELECT_PIECE) {
		switch (button) {
		case INPUT_ACCEPT:
			if ((board[player_turn][sel_piece_type] >> sel_square) & 1) {
				cur_mode = SELECT_MOVE;
				sel_move_idx = (player_turn == 0 ?
						0 : sel_piece_moves_len - 1);
			}
			break;
		case INPUT_UP:
			move_sel_square(player_turn == 0 ? 8 : -8);
			break;
		case INPUT_DOWN:
			move_sel_square(player_turn == 0 ? -8 : 8);
			break;
		case INPUT_LEFT:
			move_sel_square(player_turn == 0 ? -1 : 1);
			break;
		case INPUT_RIGHT:
			move_sel_square(player_turn == 0 ? 1 : -1);
			break;
		}
	} else if (cur_mode == SELECT_MOVE) {
		switch (button) {
		case INPUT_BACK:
			cur_mode = SELECT_PIECE;
			break;
		case INPUT_ACCEPT:
			if (sel_piece_moves_len > 0)
				move_piece();
			break;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			if (sel_piece_moves_len > 0)
				sel_move_idx = (sel_move_idx + 1) % sel_piece_moves_len;
			break;
		}
	}
}
