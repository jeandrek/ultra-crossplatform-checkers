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
#include "game_display.h"
#include "game_interaction.h"
#include "game_net.h"

enum type game_type;
enum mode cur_mode;

static void
game_init(void)
{
	board_init();
	game_display_init();
	game_interaction_init();
	if (game_net_connected()) {
		game_dirty = 1;
		game_type = NETWORK;
	} else {
		game_type = LOCAL_2PLAYER;
	}
}

static void
game_update(void)
{
	if (cur_mode == ANIM_ROTATE_BOARD) {
		game_anim();
	} else if (cur_mode == WAIT_TURN && game_net_poll_move()) {
		struct move move;
		int finished;
		game_net_recv_move(&move);
		finished = perform_move(&move, !game_net_player);
		if (finished) {
			if (winner() != -1)
				game_over();
			else
				game_interaction_turn();
		}
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

void
game_start_anim_rotate(void)
{
	cur_mode = ANIM_ROTATE_BOARD;
}

void
game_anim_rotate_finished(void)
{
	cur_mode = SELECT_PIECE;
}

static void
game_mouse_up_event(float x, float y)
{
}

static void
game_destroy(void)
{
	if (game_net_connected()) game_net_disconnect();
	game_dirty = 0;
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update,
	.destroy = game_destroy,
	.button_event = game_button_event,
	.mouse_up_event = game_mouse_up_event
};
