#ifdef __psp__
#include <pspctrl.h>

#include "input_mapping-psp.h"
#else
#include <SDL2/SDL.h>

#include "input_mapping-sdl.h"
#endif

#include "checkers.h"
#include "input.h"

static const int can_repeat[NUM_BUTTONS] = {
  [INPUT_UP] = 1,
  [INPUT_DOWN] = 1,
  [INPUT_LEFT] = 1,
  [INPUT_RIGHT] = 1
};

static int repeat_delay[NUM_BUTTONS] = {0};

static void
handle_button(int button)
{
	if (repeat_delay[button] == 0) {
		checkers_input_event(button);
		repeat_delay[button] = can_repeat[button] ? REPEAT_DELAY : -1;
	} else if (repeat_delay[button] > 0) {
		repeat_delay[button]--;
	}
}

void
input_handle(void)
{
#ifdef __psp__
	SceCtrlData data;

	sceCtrlReadBufferPositive(&data, 1);
#else
	const Uint8 *state;

	state = SDL_GetKeyboardState(NULL);
#endif
	for (int i = 0; i < NUM_BUTTONS; i++) {
		int val = input_mapping[i];
#ifdef __psp__
		if (data.Buttons & val) handle_button(i);
#else
		if (state[val]) handle_button(i);
#endif
		else repeat_delay[i] = 0;
	}
}
