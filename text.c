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

#include "checkers.h"
#include "scenegraph.h"
#include "text.h"
#include "sprite.h"
#include "texture.h"

#define FONT_ROW_LENGTH	16

struct glyph {
	int	tex_left, tex_top;
	int	width;
};

static struct texture texture_font;
static struct glyph *ascii_glyphs[128] = {NULL};

void
text_init(void)
{
	texture_init_from_file(&texture_font, 128, 128,
			       TEXTURES_DIR "font");

	for (int i = '!'; i <= '~'; i++) {
		int row = (i - '!') / FONT_ROW_LENGTH;
		int col = (i - '!') % FONT_ROW_LENGTH;
		struct glyph *g = malloc(sizeof (struct glyph));
		g->tex_left = FONT_WIDTH*col;
		g->tex_top = FONT_HEIGHT*row;
		g->width = FONT_WIDTH;
		ascii_glyphs[i] = g;
	}
}

static float scale = 1.0;
static uint32_t color = 0xffffffff;

void
text_scale(float new_scale)
{
	scale = new_scale;
}

void
text_color(uint32_t new_color)
{
	color = new_color;
}

static void
draw_glyph(struct scenegraph *scenegraph, struct glyph *g, float x, float y)
{
	struct sprite s;
	sprite_init(&s, &texture_font, g->tex_left, g->tex_top,
		    g->width, FONT_HEIGHT);
	s.scale = scale;
	s.base_color = color;
	s.x = x;
	s.y = y;
	sprite_draw(scenegraph, &s);
}

void
text_draw(struct scenegraph *scenegraph, char *str, float x, float y,
	  int alignment)
{
	float pixel_size = 2.0/scenegraph->height;
	int c;

	switch (alignment) {
	case TEXT_TOPLEFT:
		x += scale * FONT_WIDTH/2.0 * pixel_size;
		y -= scale * FONT_HEIGHT/2.0 * pixel_size;
		break;
	case TEXT_CENTRE:
		x -= scale * FONT_WIDTH/2.0 * pixel_size * (strlen(str) - 1);
		break;
	}

	while ((c = *str++)) {
		if (ascii_glyphs[c] != NULL) {
			draw_glyph(scenegraph, ascii_glyphs[c], x, y);
		}
		x += scale * FONT_WIDTH * pixel_size;
	}
}

void
text_screen_bounds(struct scenegraph *scenegraph, size_t len, float x, float y,
	       int alignment, struct rect *rect)
{
	int screen_x = scenegraph->width/2 + scenegraph->height/2 * x;
	int screen_y = scenegraph->height/2 - scenegraph->height/2 * y;

	switch (alignment) {
	case TEXT_CENTRE:
		rect->left = screen_x - scale * FONT_WIDTH/2 * len;
		rect->top = screen_y - scale * FONT_HEIGHT/2;
		rect->right = screen_x + scale * FONT_WIDTH/2 * len;
		rect->bottom = screen_y + scale * FONT_HEIGHT/2;
		break;
	case TEXT_TOPLEFT:
		rect->left = screen_x;
		rect->top = screen_y;
		rect->right = screen_x + scale * FONT_WIDTH * len;
		rect->bottom = screen_y + scale * FONT_HEIGHT;
		break;
	}
}
