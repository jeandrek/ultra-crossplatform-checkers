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

#include <stdlib.h>
#include <string.h>
#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "game_interaction.h"
#include "game_net.h"
#include "game_computer.h"
#include "text.h"

board_t cur_board;
int game_dirty;
enum type game_type;
int cur_player;
enum mode cur_mode;
enum mode anim_done_mode;
int end_turn;
int8_t *squares_buffer;

/* XXX */
float menu_button_x, menu_button_y;
struct rect menu_button_bounds;
int menu_button_highlighted;

static struct other_player *other_player = NULL;

static void menu_button_init(void);

static void
game_init(void)
{
	if (game_type == NO_GAME)
		game_type = LOCAL_2PLAYER;

	game_dirty = 0;
	cur_player = 0;
	board_init(cur_board);
	game_display_init();
#ifndef __psp__
	game_init_squares_buffer();
#endif
	if (game_net_connected()) {
		game_dirty = 1;
		game_type = NETWORK;
	}
	game_interaction_init();
	menu_button_init();

	if (game_type == NETWORK)	other_player = &other_player_net;
	else if (game_type == COMPUTER)	other_player = &other_player_computer;
}

static void
menu_button_init(void)
{
	float margin = 8 * 2.0/game.sg.height;
	menu_button_x = -game.sg.width/(float)game.sg.height + margin;
	menu_button_y = 1 - margin;
	text_screen_bounds(&game.sg, strlen("Menu"),
			   menu_button_x, menu_button_y,
			   TEXT_TOPLEFT, &menu_button_bounds);
	menu_button_bounds.left -= 8;
	menu_button_bounds.top -= 8;
	menu_button_bounds.right += 8;
	menu_button_bounds.bottom += 8;
	menu_button_highlighted = 0;
}

static void
game_update(void)
{
	if (cur_mode == ANIM_MOVE_PIECE) {
		if (!game_anim_move_piece()) {
			if (end_turn)
				cur_player = !cur_player;

			if (end_turn && winner(cur_board, cur_player) != -1)
				game_over();
			else if (end_turn && game_type == LOCAL_2PLAYER)
				cur_mode = ANIM_ROTATE_BOARD;
			else
				cur_mode = anim_done_mode;
		}
	} else if (cur_mode == ANIM_ROTATE_BOARD) {
		if (!game_anim_rotate_board())
			cur_mode = SELECT_PIECE;
	} else if (cur_mode == WAIT_TURN && other_player->poll_move()) {
		struct move move;
		int finished;

		if (!other_player->next_move(&move)) {
			cur_mode = LOST_CONNECTION;
			game_display_game_over();
			game_net_disconnect();
			return;
		}

		finished = perform_move(move.from, move.to,
					cur_board, !user_player, cur_board);
		game_dirty = 1;
		game_display_apply_move(move);
		cur_mode = ANIM_MOVE_PIECE;
		anim_done_mode = finished ? SELECT_PIECE : WAIT_TURN;
		if (finished)
			game_interaction_turn();
		else if (game_type == COMPUTER)
			game_computer_turn();
		end_turn = finished;
	}
}

void
game_over(void)
{
	cur_mode = GAME_OVER;
	game_display_game_over();
	if (game_type == NETWORK)
		game_net_disconnect();
}

static void
game_destroy(void)
{
	if (game_net_connected()) game_net_disconnect();
	if (squares_buffer) {
		free(squares_buffer);
		squares_buffer = NULL;
	}
	game_display_destroy();
	game_dirty = 0;
	game_type = NO_GAME;
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update,
	.destroy = game_destroy,
	.button_event = game_button_event,
	.mouse_up_event = game_mouse_up_event,
	.mouse_move_event = game_mouse_move_event
};
