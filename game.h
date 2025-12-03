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

#endif /* !_GAME_H_ */
