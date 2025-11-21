#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "checkers.h"
#include "game.h"
#include "menu.h"
#include "scenegraph.h"
#include "texture.h"
#include "models.h"
#include "input.h"

static struct scenegraph scenegraph;

struct texture texture_board;

static uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};
int menu = 1;

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
render_piece(struct scenegraph *scenegraph, float x, float y, float z,
	     uint32_t color)
{
	struct sg_object obj;
	obj.flags = 0;
	obj.color = color;
	obj.vertices = oct_verts;
	obj.num_vertices = 96;
	obj.x = x;
	obj.y = y;
	obj.z = z;
	sg_render_object(scenegraph, &obj);
}

void
render_pieces(struct scenegraph *scenegraph)
{
	for (int i = 0; i < 64; i++) {
		if ((board[0] >> i) & 1) {
			render_piece(scenegraph,
				     -0.875 + 0.25*(i % 8),
				     0.13, -(-0.875 + 0.25*(i / 8)),
				     0xff0000ff);
		}
	}

	for (int i = 0; i < 64; i++) {
		if ((board[1] >> i) & 1) {
			render_piece(scenegraph,
				     -0.875 + 0.25*(i % 8),
				     0.13, -(-0.875 + 0.25*(i / 8)),
				     0xff4d4d4d);
		}
	}
}

void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces
};

void
game_load(void)
{
	texture_init_from_file(&texture_board, 128, 128,
			       "assets/textures/board");
}

void
game_init(void)
{
	bzero(&scenegraph, sizeof (scenegraph));
	scenegraph.num_render = 3;
	scenegraph.render = render_functions;
	scenegraph.cam3d_enabled = 1;
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
	sg_init_scenegraph(&scenegraph);

	update = game_update;
}

void
game_update(void)
{
	if (input_read() == 4) {
		menu_init();
	}
	sg_render(&scenegraph);
}
