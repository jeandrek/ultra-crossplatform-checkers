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

#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

#define MAN	0
#define KING	1
#define NUM_PIECE_TYPES	2

typedef uint64_t board_t[2][2];

/*
 * Note: for the sake of implementation a "move" has at most 1
 * capture; hence a turn may consist of several moves.
 */

void board_init(board_t board);

/*
 * Returns piece or -1.
 */
int piece_occupying_square_belonging_to_player(board_t board, int i,
					       int player);

uint64_t piece_moves(board_t board, int player, int i,
		     int capturing);

void board_available_moves(board_t board, uint64_t moves[64],
			   int player, int moved_piece_idx);

int is_promotion(int from, int to, int type, int player);
int captured_piece_index(int from, int to);

/*
 * Performs move and returns whether the player's turn has ended yet.  Can be used
 * destructively by passing the same board twice.
 */
int perform_move(int from, int to, board_t board, int player, board_t result);

/*
 * Returns winner or -1, given a board and whose turn it is.
 */
int winner(board_t board, int player);

#endif /* !_GAME_CHECKERS_H_ */
