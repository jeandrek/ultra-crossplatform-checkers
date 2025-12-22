#include "game_checkers.h"

uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

void
board_init(void)
{
	board[0] = 0x55aa55;
	board[1] = 0xaa55aa0000000000;
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

int
piece_moves(struct move *moves, int i)
{
	int player = (board[1] >> i) & 1;
	int y = i / 8;
	int x = i % 8;
	int n = 0;
	uint64_t squares = diag_forward_squares(player, x, y);

	for (int j = 0; j < 64; j++) {
		if ((squares >> j) & 1) {
			if ((board[!player] >> j) & 1) {
				uint64_t squares2 =
					diag_forward_squares(player, j % 8,
							     j / 8);
				for (int k = 0; k < 64; k++) {
					if ((squares2 >> k) & 1)
						if (!((board[0] >> k) & 1)
						    && !((board[1] >> k) & 1)) {
							moves[n].location = k;
							moves[n].resulting_board[player] = (board[player] ^ ((uint64_t)1<<i)) | ((uint64_t)1<<k);
							moves[n].resulting_board[!player] = (board[!player] ^ ((uint64_t)1<<j));
							n++;
						}
				}
			} else if (!((board[player] >> j) & 1)) {
				moves[n].location = j;
				moves[n].resulting_board[player] = (board[player] ^ ((uint64_t)1<<i)) | ((uint64_t)1<<j);
				moves[n].resulting_board[!player] = board[!player];
				n++;
			}
		}
	}
	return n;
}
