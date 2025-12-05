#ifndef _GAME_CHECKERS_H_
#define _GAME_CHECKERS_H_

#include <stdint.h>

extern uint64_t board[2];

#define MAX_MOVES	2

void board_init(void); /* Rename? */

int piece_moves(int *moves, int i);

#endif /* !_GAME_CHECKERS_H_ */
