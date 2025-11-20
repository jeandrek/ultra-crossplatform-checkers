#ifdef __psp__
#include <pspctrl.h>
#else
#include <SDL2/SDL.h>
#endif

int
input_read(void)
{
#ifdef __psp__
	SceCtrlLatch latch;

	sceCtrlReadLatch(&latch);
	if (latch.uiBreak & PSP_CTRL_UP)	return 1;
	if (latch.uiBreak & PSP_CTRL_DOWN)	return 2;
	if (latch.uiBreak & PSP_CTRL_CROSS)	return 3;
	if (latch.uiBreak & PSP_CTRL_START)	return 4;
	return 0;
#else
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) return 1;
	if (state[SDL_SCANCODE_DOWN]) return 2;
	if (state[SDL_SCANCODE_SPACE]) return 3;
	if (state[SDL_SCANCODE_ESCAPE]) return 4;
	return 0;
#endif
}
