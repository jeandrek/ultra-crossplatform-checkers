#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

#define MAN	0
#define KING	1

typedef uint64_t board_t[2][2];

struct move {
	int	location;
	board_t	resulting_board;
};

extern board_t board;

#define MAX_MOVES	2

void board_init(void); /* Rename? */

int piece_moves(struct move *moves, int i);

void perform_move(struct move *move);

#endif /* !_GAME_CHECKERS_H_ */
