#include "input.h"

#ifdef __psp__
#include <pspctrl.h>

#include "input_mapping-psp.h"
#else
#include <SDL2/SDL.h>

#include "input_mapping-sdl.h"
#endif

int
input_read(void)
{
#ifdef __psp__
	SceCtrlLatch latch;

	sceCtrlReadLatch(&latch);
#else
	const Uint8 *state;

	state = SDL_GetKeyboardState(NULL);
#endif
	for (int i = 0; i < sizeof (input_mapping)/sizeof (int); i++) {
		int val = input_mapping[i];
#ifdef __psp__
		if (latch.uiBreak & val) return i;
#else
		if (state[val]) return i;
#endif
	}
	return 0;
}
