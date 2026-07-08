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
#include "game_computer.h"
#include "menu.h"
#include "input.h"

struct move board_moves[64][MAX_MOVES];
int board_num_moves[64];

int sel_square;
int sel_piece_type;
int sel_piece_moves_len;
struct move *sel_piece_moves;
int sel_move_idx;
int user_player = 0;

static void
set_sel_square(int i)
{
	sel_square = i;
	sel_piece_type =
		piece_occupying_square_belonging_to_player(cur_board, i,
							   user_player);
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
		user_player = game_net_player;
	else if (game_type == COMPUTER)
		user_player = !game_computer_player;
	else
		user_player = 0;

	game_display_set_viewpoint(user_player);
	if (user_player == 0) {
		cur_mode = SELECT_PIECE;
		board_available_moves(cur_board, board_moves, board_num_moves,
				      user_player, -1);
		set_sel_square(0);
	} else {
		cur_mode = WAIT_TURN;
		sel_piece_moves_len = 0;
		if (game_type == COMPUTER)
			game_computer_turn();
	}
}

void
game_interaction_turn(void)
{
	board_available_moves(cur_board, board_moves, board_num_moves,
			      user_player, -1);
	set_sel_square(user_player == 0 ? 0 : 63);
}

static void
select_piece_at_sel_square(void)
{
	if (sel_piece_moves_len > 0) {
		cur_mode = SELECT_MOVE;
		sel_move_idx = (user_player == 0 ?
				0 : sel_piece_moves_len - 1);
	}
}

static void
move_piece(void)
{
	int location = sel_piece_moves[sel_move_idx].location;
	int finished = perform_move(cur_board, &sel_piece_moves[sel_move_idx],
				    user_player);
	game_dirty = 1;
	game_display_apply_move(&sel_piece_moves[sel_move_idx]);
	if (game_type == NETWORK)
		game_net_send_move(&sel_piece_moves[sel_move_idx]);
	else if (game_type == COMPUTER)
		game_computer_turn();
	sel_move_idx = 0;
	end_turn = finished;
	if (finished) {
		if (game_type != LOCAL_2PLAYER) {
			anim_done_mode = WAIT_TURN;
			sel_piece_moves_len = 0;
		} else {
			user_player = !user_player;
			board_available_moves(cur_board, board_moves, board_num_moves,
					      user_player, -1);
			set_sel_square(user_player == 0 ? 0 : 63);
		}
	} else {
		anim_done_mode = SELECT_PIECE;
		board_available_moves(cur_board, board_moves, board_num_moves,
				      user_player, location);
		set_sel_square(location);
	}
	cur_mode = ANIM_MOVE_PIECE;
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
			select_piece_at_sel_square();
			break;
		case INPUT_UP:
			move_sel_square(user_player == 0 ? 8 : -8);
			break;
		case INPUT_DOWN:
			move_sel_square(user_player == 0 ? -8 : 8);
			break;
		case INPUT_LEFT:
			move_sel_square(user_player == 0 ? -1 : 1);
			break;
		case INPUT_RIGHT:
			move_sel_square(user_player == 0 ? 1 : -1);
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

static int
mouse_coords_to_square(int x, int y)
{
	int idx;

	if (squares_buffer == NULL || x < 0 || x >= game.sg.width
	    || y < 0 || y >= game.sg.height)
		return -1;
	idx = squares_buffer[y * game.sg.width + x];
	if (idx < 0)
		return idx;
	return user_player == 0 ? idx : 63 - idx;
}

void
game_mouse_up_event(int x, int y)
{
	int idx = mouse_coords_to_square(x, y);

	if (x >= menu_button_bounds.left && x <= menu_button_bounds.right
	    && y >= menu_button_bounds.top && y <= menu_button_bounds.bottom) {
		menu.init();
		checkers_switch_state(&menu);
		return;
	}

	if (cur_mode == SELECT_PIECE) {
		if (idx < 0) return;
		set_sel_square(idx);
		select_piece_at_sel_square();
	} else if (cur_mode == SELECT_MOVE) {
		if (idx >= 0) {
			for (int i = 0; i < sel_piece_moves_len; i++) {
				if (sel_piece_moves[i].location == idx) {
					sel_move_idx = i;
					move_piece();
					return;
				}
			}
			set_sel_square(idx);
		}
		cur_mode = SELECT_PIECE;
	}
}

void
game_mouse_move_event(int x, int y)
{
	int idx = mouse_coords_to_square(x, y);

	menu_button_highlighted =
		(x >= menu_button_bounds.left
		 && x <= menu_button_bounds.right
		 && y >= menu_button_bounds.top
		 && y <= menu_button_bounds.bottom);

	if (idx < 0) return;

	if (cur_mode == SELECT_PIECE) {
		set_sel_square(idx);
	} else if (cur_mode == SELECT_MOVE) {
		for (int i = 0; i < sel_piece_moves_len; i++) {
			if (sel_piece_moves[i].location == idx) {
				sel_move_idx = i;
				break;
			}
		}
	}
}
