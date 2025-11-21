#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "checkers.h"
#include "scenegraph.h"

static SDL_Window *window;

int
main(void)
{
	struct scenegraph scenegraph;
	SDL_GLContext context;
	SDL_Event ev;
	uint32_t ticks;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == -1) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow("Hello", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  800, 450,
				  SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);

	if (!window) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	context = SDL_GL_CreateContext(window);

	sg_init();
	bzero(&scenegraph, sizeof (scenegraph));
	scenegraph.width = 800;
	scenegraph.height = 450;
	checkers_init(&scenegraph);

	for (;;) {
		if (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
					goto quit;
				break;
			case SDL_QUIT:
				goto quit;
			}
		}
		// Run at as close to 60 FPS as possible
		if (SDL_GetTicks() - ticks >= 16) {
			checkers_update(&scenegraph);
			sg_render(&scenegraph);
			SDL_GL_SwapWindow(window);
			ticks = SDL_GetTicks();
		} else {
			SDL_Delay(2); // Idle
		}
	}
quit:
}
