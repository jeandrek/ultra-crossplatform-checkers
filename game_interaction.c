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

uint64_t board_moves[64];

int sel_square;
int sel_piece_type;
uint64_t sel_piece_moves;
int sel_move;
int user_player = 0;

static void
set_sel_square(int i)
{
	sel_square = i;
	sel_piece_type =
		piece_occupying_square_belonging_to_player(cur_board, i,
							   user_player);
	sel_piece_moves = board_moves[i];
	if (user_player == 0) {
		for (int i = 0; i < 64; i++) {
			if (sel_piece_moves & ((uint64_t)1 << i)) {
				sel_move = i;
				break;
			}
		}
	} else {
		for (int i = 63; i >= 0; i--) {
			if (sel_piece_moves & ((uint64_t)1 << i)) {
				sel_move = i;
				break;
			}
		}
	}
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
		board_available_moves(cur_board, board_moves, user_player, -1);
		set_sel_square(0);
	} else {
		cur_mode = WAIT_TURN;
		sel_piece_moves = 0;
		if (game_type == COMPUTER)
			game_computer_turn();
	}
}

void
game_interaction_turn(void)
{
	board_available_moves(cur_board, board_moves, user_player, -1);
	set_sel_square(user_player == 0 ? 0 : 63);
}

static void
select_piece_at_sel_square(void)
{
	if (sel_piece_moves > 0)
		cur_mode = SELECT_MOVE;
}

static void
move_piece(void)
{
	struct move move = { .from = sel_square, .to = sel_move };
	int finished = perform_move(sel_square, sel_move, cur_board,
				    user_player, cur_board);
	game_dirty = 1;
	game_display_apply_move(move);
	if (game_type == NETWORK)
		game_net_send_move(move);
	else if (game_type == COMPUTER)
		game_computer_turn();
	end_turn = finished;
	if (finished) {
		if (game_type != LOCAL_2PLAYER) {
			anim_done_mode = WAIT_TURN;
			sel_piece_moves = 0;
		} else {
			user_player = !user_player;
			board_available_moves(cur_board, board_moves, user_player, -1);
			set_sel_square(user_player == 0 ? 0 : 63);
		}
	} else {
		anim_done_mode = SELECT_PIECE;
		board_available_moves(cur_board, board_moves, user_player, move.to);
		set_sel_square(move.to);
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
			if (sel_piece_moves > 0)
				move_piece();
			break;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			if (sel_piece_moves > 0)
				do
					sel_move = (sel_move + 1) % 64;
				while (!(sel_piece_moves & ((uint64_t)1 << sel_move)));
			break;
		}
	}
}

static int
mouse_coords_to_square(int x, int y)
{
	int idx;

	if (squares_buffer == NULL || x < 0 || x >= sg_width
	    || y < 0 || y >= sg_height)
		return -1;
	idx = squares_buffer[y * sg_width + x];
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
			if (sel_piece_moves & ((uint64_t)1 << idx)) {
				sel_move = idx;
				move_piece();
				return;
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

	if (cur_mode == SELECT_PIECE)
		set_sel_square(idx);
	else if (cur_mode == SELECT_MOVE)
		if (sel_piece_moves & ((uint64_t)1 << idx))
			sel_move = idx;
}
