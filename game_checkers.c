#include "game_checkers.h"

struct board board;

void
board_init(void)
{
	board.men[0] = 0x55aa55;
	board.men[1] = 0xaa55aa0000000000;
	board.kings[0] = 0;
	board.kings[1] = 0;
}

static int
is_square_empty(int i)
{
	return !(((board.men[0] >> i) & 1)
		 || ((board.men[1] >> i) & 1)
		 || ((board.kings[0] >> i) & 1)
		 || ((board.kings[1] >> i) & 1));
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
	int player = (board.men[1] >> i) & 1;
	int x = i % 8;
	int y = i / 8;
	int n = 0;
	uint64_t squares = diag_forward_squares(player, x, y);

	for (int j = 0; j < 64; j++) {
		if ((squares >> j) & 1) {
			if ((board.men[!player] >> j) & 1) {
				int x2 = j % 8;
				int y2 = j / 8;
				int x3, y3;
				int k;
				y3 = y2 + (y2 - y);
				x3 = x2 + (x2 - x);
				if (y3 < 0 || y3 > 7) continue;
				if (x3 < 0 || x3 > 7) continue;
				k = 8 * y3 + x3;
				if (is_square_empty(k)) {
					moves[n].location = k;
					moves[n].resulting_board.men[player] = (board.men[player] ^ ((uint64_t)1<<i)) | ((uint64_t)1<<k);
					moves[n].resulting_board.men[!player] = (board.men[!player] ^ ((uint64_t)1<<j));
					n++;
				}
			} else if (!((board.men[player] >> j) & 1)) {
				moves[n].location = j;
				moves[n].resulting_board.men[player] = (board.men[player] ^ ((uint64_t)1<<i)) | ((uint64_t)1<<j);
				moves[n].resulting_board.men[!player] = board.men[!player];
				n++;
			}
		}
	}
	return n;
}

void
perform_move(struct move *move)
{
	board.men[0] = move->resulting_board.men[0];
	board.men[1] = move->resulting_board.men[1];
	board.kings[0] = move->resulting_board.kings[0];
	board.kings[1] = move->resulting_board.kings[1];
}
