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

#include "game_checkers.h"

void
board_init(board_t board)
{
	board[0][MAN] = 0x55aa55;
	board[1][MAN] = 0xaa55aa0000000000;
	board[0][KING] = 0;
	board[1][KING] = 0;
}

static int
is_square_empty(board_t board, int i)
{
	return !(((board[0][MAN] >> i) & 1)
		 || ((board[1][MAN] >> i) & 1)
		 || ((board[0][KING] >> i) & 1)
		 || ((board[1][KING] >> i) & 1));
}

/*
 * Returns piece or -1.
 */
int
piece_occupying_square_belonging_to_player(board_t board, int i, int player)
{
	if ((board[player][MAN] >> i) & 1) return MAN;
	if ((board[player][KING] >> i) & 1) return KING;
	return -1;
}

static uint64_t
diag_forward_squares(int player, int x, int y)
{
	uint64_t result = 0;
	if (player == 0) {
		if (x > 0 && y < 7)
			result |= (uint64_t)1 << (8 * (y + 1) + x - 1);
		if (x < 7 && y < 7)
			result |= (uint64_t)1 << (8 * (y + 1) + x + 1);
	} else {
		if (x > 0 && y > 0)
			result |= (uint64_t)1 << (8 * (y - 1) + x - 1);
		if (x < 7 && y > 0)
			result |= (uint64_t)1 << (8 * (y - 1) + x + 1);
	}
	return result;
}

static uint64_t
diag_adj_squares(int x, int y)
{
	uint64_t result = 0;
	if (x > 0 && y < 7)
		result |= (uint64_t)1 << (8 * (y + 1) + x - 1);
	if (x < 7 && y < 7)
		result |= (uint64_t)1 << (8 * (y + 1) + x + 1);
	if (x > 0 && y > 0)
		result |= (uint64_t)1 << (8 * (y - 1) + x - 1);
	if (x < 7 && y > 0)
		result |= (uint64_t)1 << (8 * (y - 1) + x + 1);
	return result;
}

#define LAST_ROW(player) ((player) == 0 ? 7 : 0)

uint64_t
piece_moves(board_t board, int player, int i, int capturing)
{
	uint64_t moves = 0;
	uint64_t squares;
	int x = i % 8;
	int y = i / 8;
	int piece;

	piece = piece_occupying_square_belonging_to_player(board, i, player);
	if (piece == -1)
		return 0;

	squares = piece == MAN ? diag_forward_squares(player, x, y) : diag_adj_squares(x, y);

	for (int j = 0; j < 64; j++) {
		if ((squares >> j) & 1) {
			if (!capturing && is_square_empty(board, j))
				moves |= (uint64_t)1 << j;
			if (capturing &&
			    piece_occupying_square_belonging_to_player(board, j, !player)
			    != -1) {
				int x2 = j % 8;
				int y2 = j / 8;
				int x3, y3;
				int k;
				y3 = y2 + (y2 - y);
				x3 = x2 + (x2 - x);
				if (y3 < 0 || y3 > 7) continue;
				if (x3 < 0 || x3 > 7) continue;
				k = 8 * y3 + x3;
				if (is_square_empty(board, k))
					moves |= (uint64_t)1 << k;
			}
		}
	}
	return moves;
}

void
board_available_moves(board_t board, uint64_t moves[64],
		      int player, int moved_piece_idx)
{
	int can_capture = 0;

	if (moved_piece_idx != -1) {
		memset(moves, 0, 64*sizeof (uint64_t));
		moves[moved_piece_idx] =
			piece_moves(board, player, moved_piece_idx, 1);
		return;
	}

	/* Available captures */
	for (int i = 0; i < 64; i++) {
		moves[i] = piece_moves(board, player, i, 1);
		if (moves[i] > 0)
			can_capture = 1;
	}

	if (can_capture)
		return;
	/* Available non-captures */
	for (int i = 0; i < 64; i++)
		moves[i] = piece_moves(board, player, i, 0);
}

int
is_promotion(int from, int to, int type, int player)
{
	return type == MAN && to / 8 == LAST_ROW(player);
}

int
captured_piece_index(int from, int to)
{
	int dist = abs(to - from);
	int sign = (to - from)/dist;
	if (dist == 14) return from + sign*7;
	if (dist == 18) return from + sign*9;
	return -1;
}

/*
 * Performs move and returns whether the player's turn has ended yet.  Can be used
 * destructively by passing the same board twice.
 */
int
perform_move(int from, int to, board_t board, int player, board_t result)
{
	int piece = piece_occupying_square_belonging_to_player(board, from, player);
	int captured = captured_piece_index(from, to);
	uint64_t mask = ~0;
	int promoted = 0;

	result[player][MAN] = board[player][MAN] & ~((uint64_t)1 << from);
	result[player][KING] = board[player][KING] & ~((uint64_t)1 << from);

	if (is_promotion(from, to, piece, player)) {
		promoted = 1;
		piece = KING;
	}
	result[player][piece] |= (uint64_t)1 << to;

	if (captured >= 0)
		mask = ~((uint64_t)1 << captured);
	result[!player][MAN] = board[!player][MAN] & mask;
	result[!player][KING] = board[!player][KING] & mask;

	return (captured < 0 || promoted ||
		piece_moves(result, player, to, 1) == 0);
}

/*
 * Returns winner or -1, given a board and whose turn it is.
 */
int
winner(board_t board, int player)
{
	uint64_t moves[64];

	if (board[0][MAN] == 0 && board[0][KING] == 0) return 1;
	if (board[1][MAN] == 0 && board[1][KING] == 0) return 0;
	board_available_moves(board, moves, player, -1);
	for (int i = 0; i < 64; i++)
		if (moves[i] > 0)
			return -1;
	return !player;
}
