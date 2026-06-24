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

#include "game.h"
#include "game_checkers.h"

int game_computer_player;

static float
evaluate(board_t board, int player)
{
	float score = 0;
	for (int i = 0; i < 64; i++) {
		if ((board[player][MAN] >> i) & 1)	score += 1;
		if ((board[player][KING] >> i) & 1)	score += 2;
		if ((board[!player][MAN] >> i) & 1)	score -= 1;
		if ((board[!player][KING] >> i) & 1)	score -= 2;
	}
	return score;
}

static struct move
search(board_t board, int player, int depth, int moved_piece_idx)
{
	struct move moves[64][MAX_MOVES];
	struct move best_move;
	float best_score = -1e10;
	int num_moves[64];
	board_available_moves_pure(board, moves, num_moves, player,
				   moved_piece_idx);
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < num_moves[i]; j++) {
			float score;
			if (depth == 0) {
				score = evaluate(moves[i][j].resulting_board,
						 player);
			} else {
				struct move *move = &moves[i][j];
				struct move next_move;
				/* XXX better to make a pure version
				   of perform_move */
				struct move further_captures[MAX_MOVES];
				int same_player = (move->capture &&
						   !move->promotion &&
						   piece_moves(board, further_captures, player, move->location, 1) > 0);
				int next_player = same_player ? player : !player;
				next_move = search(moves[i][j].resulting_board,
						    next_player, depth - 1,
						    same_player ? move->location : -1);
				score = evaluate(next_move.resulting_board,
						 player);
			}
			if (score > best_score) {
				best_score = score;
				best_move = moves[i][j];
			}
		}
	}
	return best_move;
}

int
game_computer_next_move(struct move *move)
{
	*move = search(board, game_computer_player, 6, -1);
	return 1;
}
