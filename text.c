#include <stdlib.h>
#include <string.h>
#include "scenegraph.h"
#include "text.h"
#include "sprite.h"
#include "texture.h"

#define FONT_WIDTH	8
#define FONT_HEIGHT	16
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
			       "assets/textures/font");

	for (int i = '!'; i <= '}'; i++) {
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
