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

#ifndef _GAME_H_
#define _GAME_H_

#include "checkers.h"
#include "game_checkers.h"

enum mode {
	SELECT_PIECE,
	SELECT_MOVE,
	ANIM_ROTATE_BOARD,
	ANIM_MOVE_PIECE,
	WAIT_TURN,
	GAME_OVER,
	LOST_CONNECTION
};

enum type {
	LOCAL_2PLAYER,
	NETWORK,
	COMPUTER
};

extern struct state game;
extern board_t cur_board;
extern int game_dirty;
extern enum type game_type;
extern int cur_player;
extern enum mode cur_mode;
extern enum mode anim_done_mode;
extern int end_turn;
extern int8_t *squares_buffer;

extern float menu_button_x, menu_button_y;
extern struct rect menu_button_bounds;
extern int menu_button_highlighted;

void game_over(void);

struct move {
	int	from, to;
};

struct other_player {
	int	(*poll_move)(void);
	int	(*next_move)(struct move *);
};

#endif /* !_GAME_H_ */
