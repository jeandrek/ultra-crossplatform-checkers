#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

struct board {
	uint64_t	men[2];
	uint64_t	kings[2];
};

struct move {
	int		location;
	struct board	resulting_board;
};

extern struct board board;

#define MAX_MOVES	2

void board_init(void); /* Rename? */

int piece_moves(struct move *moves, int i);

void perform_move(struct move *move);

#endif /* !_GAME_CHECKERS_H_ */
