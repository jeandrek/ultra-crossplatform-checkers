#if defined(__psp__)
#include <pspctrl.h>

#include "input_mapping-psp.h"
#elif defined(_WIN32)
#include <windows.h>

#include "input_mapping-win.h"
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

#if !defined(__psp__) && !defined(_WIN32)
extern int button_state[NUM_BUTTONS];
#endif

void
input_handle(void)
{
#if defined(__psp__)
	SceCtrlData data;

	sceCtrlReadBufferPositive(&data, 1);
#elif defined(_WIN32)
	uint8_t state[256];

	GetKeyboardState(state);
#endif
	for (int i = 0; i < NUM_BUTTONS; i++) {
#if defined(__psp__)
		int val = input_mapping[i];
		if (data.Buttons & val) handle_button(i);
#elif defined(_WIN32)
		int val = input_mapping[i];
		if (state[val] >> 7) handle_button(i);
#else
		if (button_state[i]) handle_button(i);
#endif
		else repeat_delay[i] = 0;
	}
}
