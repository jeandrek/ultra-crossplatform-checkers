#ifndef _GAME_H_
#define _GAME_H_

#include "checkers.h"

extern struct state game;
extern int game_selected_square, game_selected_move;

uint64_t piece_moves(int i);

#endif /* !_GAME_H_ */
