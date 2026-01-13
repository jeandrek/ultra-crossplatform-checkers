#ifndef _INPUT_MAPPING_H_
#define _INPUT_MAPPING_H_
#include "input.h"

static int input_mapping[] = {
	[INPUT_UP] = XK_Up,
	[INPUT_DOWN] = XK_Down,
	[INPUT_LEFT] = XK_Left,
	[INPUT_RIGHT] = XK_Right,
	[INPUT_PAUSE] = XK_Escape,
	[INPUT_ACCEPT] = XK_space,
	[INPUT_BACK] = XK_BackSpace
};

#endif /* !_INPUT_MAPPING_H_ */
