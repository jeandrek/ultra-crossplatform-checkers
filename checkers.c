#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "scenegraph.h"
#include "texture.h"
#include "models.h"

#ifndef __psp__
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif

#ifdef __psp__
#include <pspsdk.h>
#include <pspkernel.h>

PSP_MODULE_INFO("checkers", 0, 1, 1);

int
exit_callback(int arg1, int arg2, void *argp)
{
	sceKernelExitGame();
}

int
callback_thread(SceSize args, void *arg)
{
	int cbid;

	cbid = sceKernelCreateCallback("exit_callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
}
#endif

struct texture texture_board;

uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

void
render_board(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = 0xffffffff;
	obj.flags = SG_OBJ_TEXTURED;
	obj.texture = &texture_board;
	obj.vertices = board_verts;
	obj.num_vertices = sizeof (board_verts)/(8*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
}

void
render_highlight(struct scenegraph *scenegraph)
{
}

void
render_pieces(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.flags = 0;
	obj.vertices = oct_verts;
	obj.num_vertices = 96;
	obj.y = 0.13;

	obj.color = 0xff0000ff;
	for (int i = 0; i < 64; i++) {
		if ((board[0] >> i) & 1) {
			obj.x = -0.875 + 0.25*(i % 8);
			obj.z = -(-0.875 + 0.25*(i / 8));
			sg_render_object(scenegraph, &obj);
		}
	}

	obj.color = 0xff4d4d4d;
	for (int i = 0; i < 64; i++) {
		if ((board[1] >> i) & 1) {
			obj.x = -0.875 + 0.25*(i % 8);
			obj.z = -(-0.875 + 0.25*(i / 8));
			sg_render_object(scenegraph, &obj);
		}
	}
}

void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces
};

int
main(void)
{
	struct scenegraph scenegraph;
#ifndef __psp__
	uint32_t ticks;
	SDL_Event ev;
#endif
	void (*render[3])(struct scenegraph *);

#ifdef __psp__
	int thread = sceKernelCreateThread("update_thread", callback_thread,
					   0x11, 0xfa0, 0, NULL);
	if (thread >= 0)
		sceKernelStartThread(thread, 0, NULL);
#endif

	scenegraph.num_render = 3;
	scenegraph.render = render_functions;
#ifdef __psp__
	scenegraph.width = 480;
	scenegraph.height = 272;
#else
	scenegraph.width = 1280;
	scenegraph.height = 720;
#endif
	scenegraph.fov = 70.0;
	scenegraph.near_plane = 0.1;
	scenegraph.far_plane = 24;
	scenegraph.cam_x = 0;
	scenegraph.cam_y = 1.5;
	scenegraph.cam_z = 1.5;
	scenegraph.cam_dir_horiz = 0;
	scenegraph.cam_dir_vert = -M_PI/4;
	scenegraph.light0_enabled = 1;
	scenegraph.light0_x = 0;
	scenegraph.light0_y = 2;
	scenegraph.light0_z = 2;
	scenegraph.light0_color = 0xffffffff;

	sg_init(&scenegraph);

	texture_init_from_file(&texture_board, 128, 128,
			       "assets/textures/board");

	for (;;) {
#ifndef __psp__
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
		// Run at as close to 40 FPS as possible
		if (SDL_GetTicks() - ticks >= 25) {
			sg_render(&scenegraph);
			ticks = SDL_GetTicks();
		} else {
			SDL_Delay(5); // Idle
		}
#else
		sg_render(&scenegraph);
#endif
	}
quit:
}
