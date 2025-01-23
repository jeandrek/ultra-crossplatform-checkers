#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "checkers.h"
#include "menu.h"
#include "scenegraph.h"
#include "texture.h"
#include "models.h"

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
	static int t = 0;
	static int waiting = 0;
	int num = 0;
	obj.flags = 0;
	obj.vertices = oct_verts;
	obj.num_vertices = 96;
	obj.y = 0.13;
	int speed = 7;
	if (waiting == 0 && t < 80+12*speed) t++;
	else if (waiting < 200) waiting++;
	else if (t > 0) t--;
	else waiting = 0;

	obj.color = 0xff0000ff;
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
			obj.x = (1 - fac)*x0 + fac*x1;
			obj.y = (1 - fac)*y0 + fac*y1;
			obj.z = (1 - fac)*z0 + fac*z1;
			sg_render_object(scenegraph, &obj);
			num++;
		}
	}

	num=0;
	obj.color = 0xff4d4d4d;
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
			obj.x = (1 - fac)*x0 + fac*x1;
			obj.y = (1 - fac)*y0 + fac*y1;
			obj.z = (1 - fac)*z0 + fac*z1;
			sg_render_object(scenegraph, &obj);
			num++;
		}
	}
}

void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces
};

void
checkers_init(struct scenegraph *scenegraph, int width, int height)
{
	scenegraph->num_render = num_menu_render_functions;
	scenegraph->render = menu_render_functions;
	/* scenegraph->num_render = 3; */
	/* scenegraph->render = render_functions; */
	scenegraph->width = width;
	scenegraph->height = height;
	scenegraph->fov = 70.0;
	scenegraph->near_plane = 0.1;
	scenegraph->far_plane = 24;
	scenegraph->cam_x = 0;
	scenegraph->cam_y = 1.5;
	scenegraph->cam_z = 21.5;
	scenegraph->cam_dir_horiz = 0;
	scenegraph->cam_dir_vert = 0;//-M_PI/4;
	scenegraph->light0_enabled = 1;
	scenegraph->light0_x = 0;
	scenegraph->light0_y = 2;
	scenegraph->light0_z = 2;
	scenegraph->light0_color = 0xffffffff;

	sg_init(scenegraph);

	texture_init_from_file(&texture_board, 128, 128,
			       "assets/textures/board");
	menu_init();
}
