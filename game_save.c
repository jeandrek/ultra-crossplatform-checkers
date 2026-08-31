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

#include <stdint.h>
#include <stdio.h>

#include "error.h"
#include "game.h"
#include "game_checkers.h"
#include "game_save.h"

int
game_save_store_values(struct game_save *save, enum type type, int player,
		       board_t board)
{

	save->magic[0] = 0xff;
	save->magic[1] = 'C';

	save->type = type;
	save->player = player;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < NUM_PIECE_TYPES; j++) {
			uint64_t b = board[i][j];
			for (int k = 0; k < 8; k++) {
				save->board[16*i + 8*j + 7 - k] = b & 0xff;
				b >>= 8;
			}
		}
	}
	return 0;
}

int
game_save_load_values(struct game_save *save, enum type *type, int *player,
		      board_t board)
{
	if (save->magic[0] != 0xff || save->magic[1] != 'C')
		return BAD_MAGIC;

	*type = save->type;
	*player = save->player;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < NUM_PIECE_TYPES; j++) {
			board[i][j] = 0;
			for (int k = 0; k < 8; k++) {
				board[i][j] <<= 8;
				board[i][j] |= save->board[16*i + 8*j + k];
			}
		}
	}
	return 0;
}

void
game_save_description(struct game_save *save, char *desc,
		      size_t desc_size)
{
	enum type type = save->type;
	int player = save->player;

	snprintf(desc, desc_size,
		 "%s\n"
		 "%s to move",
		 type == LOCAL_2PLAYER ? "Player vs. player" :
		 player == 0 ? "Player (black) vs. computer" :
		 "Player (red) vs. computer",
		 player == 0 ? "Black" : "Red");
}
