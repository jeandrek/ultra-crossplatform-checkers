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
#include "game_computer.h"

#ifdef _WIN32
#include <windows.h>

static HANDLE turn_event;
#endif

#if defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t turn_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t turn_cond = PTHREAD_COND_INITIALIZER;
static int turn_flag = 0;
#endif

#ifdef __psp__
#include <pspkernel.h>

static int game_computer_thread;
#endif

struct other_player other_player_computer = {
	.poll_move = game_computer_poll_move,
	.next_move = game_computer_next_move
};

int game_computer_player;

static float
evaluate(board_t board, int player)
{
	float score = 0;
	int w = winner(board, !player);
	if (w != -1)
		return w == player ? 1e10 : -1e10;
	for (int i = 0; i < 64; i++) {
		if ((board[player][MAN] >> i) & 1)	score += 1;
		if ((board[player][KING] >> i) & 1)	score += 2;
		if ((board[!player][MAN] >> i) & 1)	score -= 1;
		if ((board[!player][KING] >> i) & 1)	score -= 2;
	}
	return score;
}

struct scored_move {
	float	score;
	struct	move move;
};

static struct scored_move
search(board_t board, int player, int depth, int moved_piece_idx)
{
	struct move moves[64][MAX_MOVES];
	struct scored_move best;
	int num_moves[64];
#ifdef __psp__
	sceKernelRotateThreadReadyQueue(0);
#endif
	board_available_moves(board, moves, num_moves, player,
			      moved_piece_idx);
	best.score = -1e10;
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < num_moves[i]; j++) {
			struct move *move = &moves[i][j];
			board_t new_board;
			float score;
			int ended;
			ended = move_resulting_board(move, board, player, new_board);
			if (depth == 0) {
				score = evaluate(new_board, player);
			} else {
				struct scored_move next_move;
				int next_player = ended ? !player : player;
				next_move = search(new_board,
						   next_player, depth - 1,
						   ended ? -1 : move->location);
				score = ended ? -next_move.score : next_move.score;
			}
			if (score >= best.score) {
				best.score = score;
				best.move = moves[i][j];
			}
		}
	}
	return best;
}

static struct scored_move my_move;
static int move_made = 0;

void
game_computer_turn(void)
{
#if defined(_WIN32)
	SetEvent(turn_event);
#elif defined(__unix__) || defined(__APPLE__)
	pthread_mutex_lock(&turn_mutex);
	turn_flag = 1;
	pthread_cond_signal(&turn_cond);
	pthread_mutex_unlock(&turn_mutex);
#elif defined(__psp__)
	sceKernelWakeupThread(game_computer_thread);
#else
#error Not yet supported
#endif
}

int
game_computer_poll_move(void)
{
	return move_made;
}

struct move *
game_computer_next_move(void)
{
	if (move_made) {
		move_made = 0;
		return &my_move.move;
	}
	return NULL;
}

#if defined(_WIN32)
static DWORD
engine_thread(void *arg)
#elif defined(__unix__) || defined(__APPLE__)
static void *
engine_thread(void *arg)
#elif defined(__psp__)
static int
engine_thread(SceSize args, void *arg)
#endif
{
	for (;;) {
#if defined(_WIN32)
		WaitForSingleObject(turn_event, INFINITE);
#elif defined(__unix__) || defined(__APPLE__)
		pthread_mutex_lock(&turn_mutex);
		while (!turn_flag)
			pthread_cond_wait(&turn_cond, &turn_mutex);
		turn_flag = 0;
		pthread_mutex_unlock(&turn_mutex);
#elif defined(__psp__)
		sceKernelSleepThread();
#else
#error Not yet supported
#endif
		my_move = search(cur_board, game_computer_player, 4, -1);
#ifdef __unix__
		printf("move score %f\n", my_move.score);
#endif
		move_made = 1;
	}
	return 0;
}

#ifdef _WIN32
void
game_computer_init(void)
{
	turn_event = CreateEvent(NULL, 0, 0, NULL);
	CreateThread(NULL, 0, engine_thread, NULL, 0, NULL);
}
#endif

#ifdef __psp__
void
game_computer_init(void)
{
	game_computer_thread = sceKernelCreateThread("engine_thread",
						     engine_thread,
						     0x21, 0x3000, 0, NULL);
	if (game_computer_thread >= 0)
		sceKernelStartThread(game_computer_thread, 0, NULL);
}
#endif

#if defined(__unix__) || defined(__APPLE__)
void
game_computer_init(void)
{
	pthread_t thread;

	if (pthread_create(&thread, NULL, engine_thread, NULL) < 0) {
		perror("checkers");
		exit(1);
	}
}
#endif
