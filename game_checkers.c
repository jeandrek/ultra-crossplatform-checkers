#include "game_checkers.h"

uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

uint64_t
piece_moves(int i)
{
	uint64_t moves = 0;
	int y = i/8;
	int x = i%8;
	if (x > 0 && y < 7)
		moves |= (uint64_t)1<<(8*(y + 1) + x - 1);
	if (x < 7 && y < 7)
		moves |= (uint64_t)1<<(8*(y + 1) + x + 1);
	return moves;
}
