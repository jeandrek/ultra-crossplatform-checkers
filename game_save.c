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
game_save_write(const char *path, enum type type, int player,
		board_t board)
{
	FILE *f = fopen(path, "wb");

	if (f == NULL)
		return CANNOT_OPEN_FILE;
	putc(0xff, f);
	putc('C', f);

	putc(type, f);
	putc(player, f);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < NUM_PIECE_TYPES; j++) {
			uint64_t b = board[i][j];
			putc(b >> 56, f);
			putc((b >> 48) & 0xff, f);
			putc((b >> 40) & 0xff, f);
			putc((b >> 32) & 0xff, f);
			putc((b >> 24) & 0xff, f);
			putc((b >> 16) & 0xff, f);
			putc((b >> 8) & 0xff, f);
			putc(b & 0xff, f);
		}
	}

	fclose(f);
	return 0;
}

int
game_save_read(const char *path, enum type *type, int *player,
	       board_t board)
{
	FILE *f = fopen(path, "rb");

	if (f == NULL)
		return CANNOT_OPEN_FILE;
	if (getc(f) != 0xff || getc(f) != 'C')
		return BAD_MAGIC;

	*type = getc(f);
	*player = getc(f);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < NUM_PIECE_TYPES; j++) {
			board[i][j] = 0;
			for (int k = 0; k < 8; k++) {
				board[i][j] <<= 8;
				board[i][j] |= getc(f);
			}
		}
	}

	fclose(f);
	return 0;
}
