#include "game_checkers.h"

board_t board;

void
board_init(void)
{
	board[0][MAN] = 0x55aa55;
	board[1][MAN] = 0xaa55aa0000000000;
	board[0][KING] = 0;
	board[1][KING] = 0;
}

static int
is_square_empty(int i)
{
	return !(((board[0][MAN] >> i) & 1)
		 || ((board[1][MAN] >> i) & 1)
		 || ((board[0][KING] >> i) & 1)
		 || ((board[1][KING] >> i) & 1));
}

int
piece_occupying_square_belonging_to_player(int i, int player)
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

static void
move_go_to_square(struct move *move, int player, int piece, int i, int j)
{
	move->location = j;
	if (piece == MAN && j / 8 == LAST_ROW(player)) {
		/* Promotion */
		move->resulting_board[player][MAN] =
			board[player][MAN] ^ (uint64_t)1<<i;
		move->resulting_board[player][KING] =
			board[player][KING] | (uint64_t)1<<j;
	} else {
		move->resulting_board[player][piece] =
			(board[player][piece] ^ (uint64_t)1<<i) | (uint64_t)1<<j;
		move->resulting_board[player][!piece] = board[player][!piece];
	}
	move->resulting_board[!player][piece] = board[!player][piece];
	move->resulting_board[!player][!piece] = board[!player][!piece];
}

static void
move_capture(struct move *move, int player, int capturing, int captured,
	     int i, int j, int k)
{
	move_go_to_square(move, player, capturing, i, k);
	move->resulting_board[!player][captured] ^= (uint64_t)1<<j;
}

static int
piece_moves(struct move *moves, int player, int i, int capturing)
{
	int piece;
	uint64_t squares;
	int x = i % 8;
	int y = i / 8;
	int n = 0;

	piece = piece_occupying_square_belonging_to_player(i, player);
	if (piece == -1)
		return 0;

	squares = piece == MAN ? diag_forward_squares(player, x, y) : diag_adj_squares(x, y);

	for (int j = 0; j < 64; j++) {
		if ((squares >> j) & 1) {
			int other;
			if (capturing && (other = piece_occupying_square_belonging_to_player(j, !player)) != -1) {
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
					move_capture(&moves[n], player, piece,
						     other, i, j, k);
					n++;
				}
			} else if (!capturing && piece_occupying_square_belonging_to_player(j, player) == -1) {
				move_go_to_square(&moves[n], player, piece, i, j);
				n++;
			}
		}
	}
	return n;
}

void
board_available_moves(struct move moves[64][MAX_MOVES], int *num_moves,
		      int player)
{
	int can_capture = 0;

	/* Available captures */
	for (int i = 0; i < 64; i++) {
		num_moves[i] = piece_moves(moves[i], player, i, 1);
		if (num_moves[i] > 0)
			can_capture = 1;
	}

	if (can_capture)
		return;
	/* Available non-captures */
	for (int i = 0; i < 64; i++) {
		num_moves[i] = piece_moves(moves[i], player, i, 0);
	}
}

void
perform_move(struct move *move)
{
	board[0][MAN] = move->resulting_board[0][MAN];
	board[0][KING] = move->resulting_board[0][KING];
	board[1][MAN] = move->resulting_board[1][MAN];
	board[1][KING] = move->resulting_board[1][KING];
}
