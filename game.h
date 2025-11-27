#ifndef _GAME_H_
#define _GAME_H_

#include "checkers.h"

extern struct state game;
extern int game_sel_square, game_sel_move;
extern uint64_t game_sel_piece_moves;

uint64_t piece_moves(int i);

#endif /* !_GAME_H_ */
