/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
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

static const uint32_t colors[2][2] = {
	{COLOR_PLAYER_0, COLOR_PLAYER_0_SEL},
	{COLOR_PLAYER_1, COLOR_PLAYER_1_SEL}
};

struct piece {
	//struct	sg_obj obj;
	float	x, y, z;
	int	location;
	int	player;
	int	type;
	struct	piece *prev;
	struct	piece *next;
};

static struct texture texture_board;
static struct sprite overlay_sprite;
static struct piece *pieces = NULL;

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
	if (!(cur_mode == SELECT_PIECE || cur_mode == SELECT_MOVE))
		return;
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
	for (struct piece *piece = pieces; piece != NULL; piece = piece->next) {
		int color = (piece->player == player_turn
			     && piece->location == sel_square
			     ? colors[piece->player][1]
			     : colors[piece->player][0]);
		render_piece(scenegraph, piece->type,
			     piece->x, piece->y, piece->z, color);
	}

	for (int i = 0; i < sel_piece_moves_len; i++) {
		float x, y, z;
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
render_status(struct scenegraph *scenegraph)
{
	char *text = NULL;

	if (cur_mode == GAME_OVER)
		text = winner() == 0 ? "Red wins" : "Black wins";
	else if (cur_mode == LOST_CONNECTION)
		text = "Lost connection";

	if (text != NULL) {
		sprite_draw(scenegraph, &overlay_sprite);
		text_size(1);
		text_color(0xffffffff);
		text_draw(scenegraph, text, 0, 0, TEXT_CENTRE);
	} else {
		float margin = (FONT_HEIGHT/2 + 8) * 2.0/game.sg.height;
		int player = cur_mode == WAIT_TURN ? !player_turn : player_turn;
		text = player == 0 ? "Red to move" : "Black to move";
		text_size(1);
		text_color(0xffaaaaaa);
		text_draw(scenegraph, text, 0, 1 - margin, TEXT_CENTRE);
	}
}

static void
render_menu_button(struct scenegraph *scenegraph)
{
	text_size(1);
	text_color(menu_button_highlighted ? 0xffffffff : 0xffaaaaaa);
	text_draw(scenegraph, "Menu", menu_button_x, menu_button_y, TEXT_TOPLEFT);
}

static void (*render_functions[])(struct scenegraph *) = {
	render_board,
	render_highlight,
	render_pieces,
	render_status,
#ifndef __psp__
	render_menu_button
#endif
};

void
game_display_load(void)
{
	texture_init_from_file(&texture_board, 128, 128,
			       TEXTURES_DIR "board");
}

static void
add_piece(int location, float x, float y, float z, int player, int type)
{
	struct piece *piece;
	piece = malloc(sizeof (*piece));
	piece->location = location;
	piece->x = x;
	piece->y = y;
	piece->z = z;
	piece->player = player;
	piece->type = type;
	piece->prev = NULL;
	piece->next = pieces;
	if (pieces != NULL) pieces->prev = piece;
	pieces = piece;
}

static struct piece *
piece_at_location(int location)
{
	for (struct piece *piece = pieces; piece; piece = piece->next)
		if (piece->location == location)
			return piece;
	return NULL;
}

static void
delete_piece(struct piece *piece)
{
	if (piece->prev != NULL)
		piece->prev->next = piece->next;
	else
		pieces = piece->next;
	if (piece->next != NULL)
		piece->next->prev = piece->prev;
	free(piece);
}

void
game_display_init(void)
{
	float x, y, z;

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
	sg_init_scenegraph(&game.sg);

	for (int player = 0; player < 2; player++) {
		for (int type = 0; type < NUM_PIECE_TYPES; type++) {
			for (int i = 0; i < 64; i++) {
				if ((cur_board[player][type] >> i) & 1) {
					board_pos_to_world_pos(&x, &y, &z, i);
					add_piece(i, x, y, z, player, type);
				}
			}
		}
	}
}

void
game_display_destroy(void)
{
	while (pieces != NULL) {
		struct piece *piece = pieces;
		pieces = pieces->next;
		free(piece);
	}
}

void
game_display_game_over(void)
{
	sprite_init(&overlay_sprite, NULL, 0, 0,
		    game.sg.width, game.sg.height);
	overlay_sprite.base_color = 0x70000000;
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
game_display_apply_move(struct move *move)
{
	struct piece *piece;

	piece = piece_at_location(move->from);

	piece->location = move->location;
	board_pos_to_world_pos(&piece->x, &piece->y, &piece->z, move->location);
	if (move->promotion)
		piece->type = KING;

	if (move->captured > 0)
		delete_piece(piece_at_location(move->captured));
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

static void
world_pos_to_screen_pos(float x_w, float y_w, float z_w, int *x_s, int *y_s)
{
	float tan_fov_2 = tanf(game.sg.fov/2.0 * M_PI/180.0);
	float aspect = (float)game.sg.width/(float)game.sg.height;
	float x_e = x_w;
	float y_e = (y_w - 1.5) * cos(M_PI/4) - (z_w - 1.5) * sin(M_PI/4);
	float z_e = (z_w - 1.5) * cos(M_PI/4) + (y_w - 1.5) * sin(M_PI/4);
	float x_c = x_e * (2*game.sg.near_plane)/(0.2*tan_fov_2*aspect);
	float y_c = y_e * (2*game.sg.near_plane)/(0.2*tan_fov_2);
	*x_s = (x_c / -z_e + 1.0f) * game.sg.width/2;
	*y_s = (-y_c / -z_e + 1.0f) * game.sg.height/2;
	/* glGetFloatv(GL_MODELVIEW_MATRIX, modelview); */
	/* glGetFloatv(GL_PROJECTION_MATRIX, projection); */
}

void
game_init_squares_buffer(void)
{
	size_t len = game.sg.width * game.sg.height * sizeof (int);
	squares_buffer = malloc(len);
	memset(squares_buffer, -1, len);
	for (int i = 0; i < 64; i++) {
		float world_top = 1 - (i/8)/4.0;
		float world_bottom = 1 - (i/8 + 1)/4.0;
		float world_left = -1 + (i%8)/4.0;
		float world_right = -1 + (i%8 + 1)/4.0;
		int top, bottom, x1, x2, x3, x4;
		world_pos_to_screen_pos(world_left, .1, world_top, &x1, &top);
		world_pos_to_screen_pos(world_right, .1, world_top, &x2, &top);
		world_pos_to_screen_pos(world_left, .1, world_bottom, &x3, &bottom);
		world_pos_to_screen_pos(world_right, .1, world_bottom, &x4, &bottom);
		for (int y = bottom; y <= top; y++) {
			int left = x3 + (y - bottom)/(float)(top - bottom) * (x1 - x3);
			int right = x4 + (y - bottom)/(float)(top - bottom) * (x2 - x4);
			for (int x = left; x <= right; x++)
				squares_buffer[y*game.sg.width + x] = i;
		}
	}
}
