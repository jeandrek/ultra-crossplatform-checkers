#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "game.h"
#include "game_display.h"
#include "game_interaction.h"
#include "game_checkers.h"
#include "scenegraph.h"
#include "texture.h"
#include "models.h"
#include "sprite.h"
#include "text.h"

#define COLOR_PLAYER_0		0xff0000ff
#define COLOR_PLAYER_0_SEL	0xff8080ff
#define COLOR_PLAYER_1		0xff4d4d4d
#define COLOR_PLAYER_1_SEL	0xffa6a6a6

#define HALF_ALPHA(col)		(((col) & 0xffffff) | (0x80 << 24))

static struct texture texture_board, texture_win_red, texture_win_black;
static struct sprite overlay_sprite, win_sprite;

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
	obj.x = -0.875 + 0.25*(sel_square % 8);
	obj.y = 0.1;
	obj.z = -(-0.875 + 0.25*(sel_square / 8));
	sg_render_object(scenegraph, &obj);
}

static void
render_piece(struct scenegraph *scenegraph, int piece_type,
	     float x, float y, float z, uint32_t color)
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
	if (piece_type == KING) {
		obj.y += 0.065;
		sg_render_object(scenegraph, &obj);
	}
}

static void
board_pos_to_world_pos(float *x, float *y, float *z, int i)
{
	*x = -0.875 + 0.25*(i % 8);
	*y = 0.13;
	*z = -(-0.875 + 0.25*(i / 8));
}

static void
render_pieces(struct scenegraph *scenegraph)
{
	float x, y, z;

	for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
		for (int i = 0; i < 64; i++) {
			if ((board[0][piece] >> i) & 1) {
				int color = (player_turn == 0 && i == sel_square ?
					     COLOR_PLAYER_0_SEL : COLOR_PLAYER_0);
				board_pos_to_world_pos(&x, &y, &z, i);
				render_piece(scenegraph, piece, x, y, z, color);
			}
		}
	}

	for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
		for (int i = 0; i < 64; i++) {
			if ((board[1][piece] >> i) & 1) {
				int color = (player_turn == 1 && i == sel_square ?
					     COLOR_PLAYER_1_SEL : COLOR_PLAYER_1);
				board_pos_to_world_pos(&x, &y, &z, i);
				render_piece(scenegraph, piece, x, y, z, color);
			}
		}
	}

	for (int i = 0; i < sel_piece_moves_len; i++) {
		int color;
		if (player_turn == 0)
			color = (cur_mode == SELECT_MOVE && i == sel_move_idx ?
				 COLOR_PLAYER_0_SEL : COLOR_PLAYER_0);
		else
			color = (cur_mode == SELECT_MOVE && i == sel_move_idx ?
				 COLOR_PLAYER_1_SEL : COLOR_PLAYER_1);
		color = HALF_ALPHA(color);
		board_pos_to_world_pos(&x, &y, &z, sel_piece_moves[i].location);
		render_piece(scenegraph, sel_piece_type, x, y, z, color);
	}
}

static void
render_win(struct scenegraph *scenegraph)
{
	if (cur_mode == GAME_OVER) {
		sprite_draw(scenegraph, &overlay_sprite);
		sprite_draw(scenegraph, &win_sprite);
	}
}

static void
render_text(struct scenegraph *scenegraph)
{
	text_scale(1);
	text_color(0xffffffff);
	draw_text(scenegraph, "Hello there... :)",
		  -4.0/3.0, 1,
		  TEXT_TOPLEFT);
}

static void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces,
	render_win,
	render_text
};

void
game_display_load(void)
{
	texture_init_from_file(&texture_board, 128, 128,
			       "assets/textures/board");
	texture_init_from_file(&texture_win_red, 128, 64,
			       "assets/textures/win-red");
	texture_init_from_file(&texture_win_black, 128, 64,
			       "assets/textures/win-black");
}

void
game_display_init(void)
{
	memset(&game.sg, 0, sizeof (game.sg));
	game.sg.num_render = sizeof (render_functions)/sizeof (render_functions[1]);
	game.sg.render = render_functions;
	game.sg.cam3d_enabled = 1;
	game.sg.fov = 70.0;
	game.sg.near_plane = 0.1;
	game.sg.far_plane = 24;
	game.sg.light0_enabled = 1;
	game.sg.light0_x = 1;
	game.sg.light0_y = 2;
	game.sg.light0_z = 0;
	game.sg.light0_color = 0xffffffff;
	game_display_set_viewpoint(0);
	sg_init_scenegraph(&game.sg);
}

void
game_display_game_over(void)
{
	sprite_init(&overlay_sprite, NULL, 0, 0,
		    game.sg.width, game.sg.height);
	overlay_sprite.base_color = 0x70000000;
	sprite_init(&win_sprite,
		    winner() == 0 ? &texture_win_red : &texture_win_black,
		    0, 0, 128, 64);
}

static int anim_ticks = 0;

void
game_anim(void)
{
	anim_ticks++;
	game.sg.cam_dir_horiz += 1 / 30.0 * M_PI;
	game.sg.cam_x = 1.5 * sinf(game.sg.cam_dir_horiz);
	game.sg.cam_z = 1.5 * cosf(game.sg.cam_dir_horiz);
	if (anim_ticks == 30) {
		anim_ticks = 0;
		game_anim_rotate_finished();
	}
}

void
game_display_set_viewpoint(int player)
{
	if (player == 0) {
		game.sg.cam_x = 0;
		game.sg.cam_y = 1.5;
		game.sg.cam_z = 1.5;
		game.sg.cam_dir_horiz = 0;
		game.sg.cam_dir_vert = -M_PI/4;
	} else {
		game.sg.cam_x = 0;
		game.sg.cam_y = 1.5;
		game.sg.cam_z = -1.5;
		game.sg.cam_dir_horiz = M_PI;
		game.sg.cam_dir_vert = -M_PI/4;
	}
}
