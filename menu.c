#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "checkers.h"
#include "menu.h"
#include "scenegraph.h"
#include "texture.h"

struct texture texture_logo;
struct texture texture_button1;
struct texture texture_button2;

static uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

static float __attribute__((aligned(16))) logo_verts[] = {
	0, 0,
	0, 0, 0,
	0, 128,
	0, 50, 0,
	256, 0,
	100, 0, 0,
	256, 0,
	100, 0, 0,
	0, 128,
	0, 50, 0,
	256, 128,
	100, 50, 0
};
static float __attribute__((aligned(16))) button1_verts[] = {
	0, 0,
	0, 0, 0,
	0, 64,
	0, 30, 0,
	128, 0,
	60, 0, 0,
	128, 0,
	60, 0, 0,
	0, 64,
	0, 30, 0,
	128, 64,
	60, 30, 0
};
static float __attribute__((aligned(16))) button2_verts[] = {
	0, 0,
	0, 0, 0,
	0, 64,
	0, 30, 0,
	128, 0,
	60, 0, 0,
	128, 0,
	60, 0, 0,
	0, 64,
	0, 30, 0,
	128, 64,
	60, 30, 0
};

void
menu_render_logo(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = ~0;
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED | SG_OBJ_NOLIGHTING;
	obj.texture = &texture_logo;
	obj.vertices = logo_verts;
	obj.num_vertices = sizeof (logo_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
}
int selected_button = 0;
void
menu_render_buttons(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = (selected_button == 0 ? ~0 : 0xffaaaaaa);
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED;
	obj.texture = &texture_button1;
	obj.vertices = button1_verts;
	obj.num_vertices = sizeof (button1_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
	obj.color = (selected_button == 1 ? ~0 : 0xffaaaaaa);
	obj.texture = &texture_button2;
	obj.vertices = button2_verts;
	sg_render_object(scenegraph, &obj);
}


void
menu_render_pieces(struct scenegraph *scenegraph)
{
	static int t = 0;
	static int waiting = 0;
	int num = 0;
	int speed = 7;
	if (waiting == 0 && t < 80+12*speed) t++;
	else if (waiting < 200) waiting++;
	else if (t > 0) t--;
	else waiting = 0;

	for (int i = 0; i < 64; i++) {
		if ((board[0] >> i) & 1) {
			float x0 = -0.875;
			float y0 = 0.13 + 0.07*(11 - num);
			float z0 = 0;
			float x1 = -0.875 + 0.25*(i % 8);
			float y1 = 0.13;
			float z1 = -(-0.875 + 0.25*(i / 8));
			int t_thres = 80 + speed*num;
			float fac = (t < t_thres ? 0 : (t > t_thres + speed ? 1 : (t - t_thres)/(float)speed));
			render_piece(scenegraph,
				     (1 - fac)*x0 + fac*x1,
				     (1 - fac)*y0 + fac*y1,
				     (1 - fac)*z0 + fac*z1,
				     0xff0000ff);
			num++;
		}
	}

	num=0;
	for (int i = 0; i < 64; i++) {
		if ((board[1] >> i) & 1) {
			float x0 = 0.875;
			float y0 = 0.13 + 0.07*(11 - num);
			float z0 = 0;
			float x1 = -0.875 + 0.25*(i % 8);
			float y1 = 0.13;
			float z1 = -(-0.875 + 0.25*(i / 8));
			int t_thres = 80 + speed*num;
			float fac = (t < t_thres ? 0 : (t > t_thres + speed ? 1 : (t - t_thres)/(float)speed));
			render_piece(scenegraph,
				     (1 - fac)*x0 + fac*x1,
				     (1 - fac)*y0 + fac*y1,
				     (1 - fac)*z0 + fac*z1,
				     0xff4d4d4d);
			num++;
		}
	}
}

void (*menu_render_functions[])(struct scenegraph *) = {
	render_board,
	menu_render_pieces,
	menu_render_logo,
	menu_render_buttons
};

size_t num_menu_render_functions = 4;


void
menu_init(void)
{
	for (int i = 2; i < 30; i += 5) {
		logo_verts[i] += 180;
		logo_verts[i+1] += 40;

		button1_verts[i] += 200;
		button1_verts[i+1] += 100;

		button2_verts[i] += 200;
		button2_verts[i+1] += 140;
	}
#ifndef __psp__
	for (int i = 0; i < 30; i += 5) {
		logo_verts[i] /= 256.0;
		logo_verts[i+1] /= 128.0;
		logo_verts[i+2] /= 240.0;
		logo_verts[i+2] -= 1;
		logo_verts[i+3] /= -136.0;
		logo_verts[i+3] += 1;
		button1_verts[i] /= 128.0;
		button1_verts[i+1] /= 64.0;
		button1_verts[i+2] /= 240.0;
		button1_verts[i+2] -= 1;
		button1_verts[i+3] /= -136.0;
		button1_verts[i+3] += 1;
		button2_verts[i] /= 128.0;
		button2_verts[i+1] /= 64.0;
		button2_verts[i+2] /= 240.0;
		button2_verts[i+2] -= 1;
		button2_verts[i+3] /= -136.0;
		button2_verts[i+3] += 1;
	}
#endif
	texture_init_from_file(&texture_logo, 256, 128,
			       "assets/textures/logo");
	texture_init_from_file(&texture_button1, 128, 64,
			       "assets/textures/menu1");
	texture_init_from_file(&texture_button2, 128, 64,
			       "assets/textures/menu2");
}
