#ifndef _GAME_H_
#define _GAME_H_

#include "checkers.h"

enum mode {
	SELECT_PIECE,
	SELECT_MOVE,
	ANIM_ROTATE_BOARD,
	ANIM_DONE
};

extern struct state game;
extern enum mode cur_mode;

void game_start_anim_rotate(void);
void game_anim_rotate_finished(void);

#endif /* !_GAME_H_ */
