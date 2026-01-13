#ifndef _INPUT_MAPPING_H_
#define _INPUT_MAPPING_H_
#include "input.h"

static int input_mapping[] = {
	[INPUT_UP] = VK_UP,
	[INPUT_DOWN] = VK_DOWN,
	[INPUT_LEFT] = VK_LEFT,
	[INPUT_RIGHT] = VK_RIGHT,
	[INPUT_PAUSE] = VK_ESCAPE,
	[INPUT_ACCEPT] = VK_SPACE,
	[INPUT_BACK] = VK_BACK
};

#endif /* !_INPUT_MAPPING_H_ */
