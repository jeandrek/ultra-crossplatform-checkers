#ifndef _GAME_INTERACTION_H_
#define _GAME_INTERACTION_H_

#include "game_checkers.h"

extern int sel_square;
extern int sel_piece_moves_len;
extern struct move sel_piece_moves[MAX_MOVES];
extern int sel_move_idx;
extern int player_turn;

void game_interaction_init(void);
void game_input_event(int button);

#endif
