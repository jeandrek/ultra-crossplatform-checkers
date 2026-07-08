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

struct __attribute__ ((packed)) move {
	unsigned int	from		: 6;
	unsigned int	location	: 6;
	int		captured	: 7;
	unsigned int	promotion	: 1;
};

#define MAX_MOVES	4

void board_init(board_t board);

/*
 * Returns piece or -1.
 */
int piece_occupying_square_belonging_to_player(board_t board, int i,
					       int player);

int piece_moves(board_t board, struct move *moves, int player, int i,
		int capturing);

void board_available_moves(board_t board, struct move moves[64][MAX_MOVES],
			   int *num_moves, int player,
			   int moved_piece_idx);

int move_resulting_board(struct move *move, board_t board, int player,
			 board_t result);

/*
 * Performs move and returns whether the player's turn has ended yet.
 */
int perform_move(board_t board, struct move *move, int player);

/*
 * Returns winner or -1, given a board, available moves, and whose turn it is.
 */
int winner(board_t board, int player);
//struct move moves[64][MAX_MOVES]);
	   //	   int *num_moves, int player);

#endif /* !_GAME_CHECKERS_H_ */
