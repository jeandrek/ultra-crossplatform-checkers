#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

#define MAN	0
#define KING	1
#define NUM_PIECE_TYPES	2

typedef uint64_t board_t[2][2];

/*
 * Note: for the sake of implementation a "move" has at most 1
 * capture; hence a turn may consist of several moves.
 */

struct move {
	int	location;
	int	capture;
	int	promotion;
	board_t	resulting_board;
};

extern board_t board;

#define MAX_MOVES	4

void board_init(void); /* Rename? */

int piece_occupying_square_belonging_to_player(int i, int player);
void board_available_moves(struct move moves[64][MAX_MOVES], int *num_moves,
			   int player, int moved_piece_idx);

int perform_move(struct move *move, int player);

#endif /* !_GAME_CHECKERS_H_ */
