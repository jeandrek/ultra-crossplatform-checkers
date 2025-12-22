#include "game_checkers.h"

uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

void
board_init(void)
{
	board[0] = 0x55aa55;
	board[1] = 0xaa55aa0000000000;
}

int
piece_moves(int *moves, int i)
{
	int n = 0;
	int y = i / 8;
	int x = i % 8;
	uint64_t diag_forward_squares = 0;

	if ((board[0] >> i) & 1) {
		if (x > 0 && y < 7)
			diag_forward_squares |= (uint64_t)1 << (8 * (y + 1) + x - 1);
		if (x < 7 && y < 7)
			diag_forward_squares |= (uint64_t)1 << (8 * (y + 1) + x + 1);
	} else if ((board[1] >> i) & 1) {
		if (x > 0 && y > 0)
			diag_forward_squares |= (uint64_t)1 << (8 * (y - 1) + x - 1);
		if (x < 7 && y > 0)
			diag_forward_squares |= (uint64_t)1 << (8 * (y - 1) + x + 1);
	}

	for (int j = 0; j < 64; j++) {
		if ((diag_forward_squares >> j) & 1) {
			if (!((board[0] >> j) & 1) && !((board[1] >> j) & 1))
				moves[n++] = j;
		}
	}
	return n;
}
