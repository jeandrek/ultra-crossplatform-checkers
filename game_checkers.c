#include "game_checkers.h"

uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

int
piece_moves(int *moves, int i)
{
	int n = 0;
	int y = i / 8;
	int x = i % 8;
	int y2;

	if ((board[0] >> i) & 1)
		y2 = y + 1;
	else if ((board[1] >> i) & 1)
		y2 = y - 1;
	else
		return 0;

	if (x > 0 && y < 7)
		moves[n++] = 8 * y2 + x - 1;
	if (x < 7 && y < 7)
		moves[n++] = 8 * y2 + x + 1;
	return n;
}
