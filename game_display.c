#include <string.h>
#include <math.h>

#include "game.h"
#include "game_display.h"
#include "scenegraph.h"
#include "texture.h"
#include "models.h"

static uint64_t board[2] = {0x55aa55, 0xaa55aa0000000000};

static struct texture texture_board;

static void
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

static void
render_highlight(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = 0x8080ffff;
	obj.flags = SG_OBJ_NOLIGHTDEPTH;
	obj.vertices = highlight_verts;
	obj.num_vertices = sizeof (highlight_verts)/(6*sizeof (float));
	obj.x = -0.875 + 0.25*(game_selected_square % 8);
	obj.y = 0.1;
	obj.z = -(-0.875 + 0.25*(game_selected_square / 8));
	sg_render_object(scenegraph, &obj);
}

static void
render_piece(struct scenegraph *scenegraph, float x, float y, float z,
	     uint32_t color)
{
	struct sg_object obj;
	obj.color = color;
	obj.flags = 0;
	obj.vertices = oct_verts;
	obj.num_vertices = 96;
	obj.x = x;
	obj.y = y;
	obj.z = z;
	sg_render_object(scenegraph, &obj);
}

static void
render_pieces(struct scenegraph *scenegraph)
{
	for (int i = 0; i < 64; i++) {
		if ((board[0] >> i) & 1) {
			render_piece(scenegraph,
				     -0.875 + 0.25*(i % 8),
				     0.13, -(-0.875 + 0.25*(i / 8)),
				     (i == game_selected_square ?
				      0xff8080ff : 0xff0000ff));
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

static void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces
};

void
game_display_load(void)
{
	texture_init_from_file(&texture_board, 128, 128,
			       "assets/textures/board");
}

void
game_display_init(void)
{
	bzero(&game.sg, sizeof (game.sg));
	game.sg.num_render = 3;
	game.sg.render = render_functions;
	game.sg.cam3d_enabled = 1;
	game.sg.fov = 70.0;
	game.sg.near_plane = 0.1;
	game.sg.far_plane = 24;
	game.sg.cam_x = 0;
	game.sg.cam_y = 1.5;
	game.sg.cam_z = 1.5;
	game.sg.cam_dir_horiz = 0;
	game.sg.cam_dir_vert = -M_PI/4;
	game.sg.light0_enabled = 1;
	game.sg.light0_x = 0;
	game.sg.light0_y = 2;
	game.sg.light0_z = 2;
	game.sg.light0_color = 0xffffffff;
	sg_init_scenegraph(&game.sg);
}
