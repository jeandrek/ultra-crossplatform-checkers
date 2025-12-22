#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

struct move {
	int		location;
	uint64_t	resulting_board[2];
};

extern uint64_t board[2];

#define MAX_MOVES	2

void board_init(void); /* Rename? */

int piece_moves(struct move *moves, int i);

void perform_move(struct move *move);

#endif /* !_GAME_CHECKERS_H_ */
