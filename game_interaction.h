#ifndef _GAME_INTERACTION_H_
#define _GAME_INTERACTION_H_

extern int game_sel_square, game_sel_move;
extern uint64_t game_sel_piece_moves;

void game_interaction_init(void);
void game_input_event(int button);

#endif
