#include <string.h>

#include "game.h"
#include "game_checkers.h"

int turns_since_promotion = 0;
int turns_since_capture = 0;

/*
#define NUM_PAST_BOARDS	100

board_t past_boards[NUM_PAST_BOARDS];
int	past_boards_idx = 0;
*/

void
game_draw_init(void)
{
	turns_since_promotion = 0;
	turns_since_capture = 0;
}

void
game_draw_update(int promotion, int capture)
{
	if (promotion) turns_since_promotion = 0;
	else turns_since_promotion++;

	if (capture) turns_since_capture = 0;
	else turns_since_capture++;
	/*
	memcpy(past_boards[past_boards_idx++], cur_board, sizeof (board_t));
	if (past_boards_idx > NUM_PAST_BOARDS) past_boards_idx = 0;
	*/
}

int
game_draw_check(void)
{
	if (turns_since_promotion == 2*40) return 1;
	if (turns_since_capture == 2*40) return 1;
	return 0;

	/*
	uint64_t moves[64];

	board_available_moves(cur_board, moves, cur_player, -1);
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			int ended, seen_before;
			board_t result;
			if (!((moves[i] >> j) & 1)) continue;
			ended = perform_move(i, j, cur_board, cur_player, result);
			if (!ended) continue;
			seen_before = 0;
			for (int k = 0; k < NUM_PAST_BOARDS; k++) {
				if (memcmp(past_boards[k], result,
					   sizeof (board_t))) {
					seen_before = 1;
					break;
				}
			}
			if (!seen_before) return 0;
		}
	}
	return 1;
	*/
}
