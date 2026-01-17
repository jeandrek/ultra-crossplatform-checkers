#include <stdlib.h>
#include <string.h>
#include "scenegraph.h"
#include "text.h"
#include "sprite.h"
#include "texture.h"

static struct texture texture_font;
static struct sprite *ascii_sprites[128] = {NULL};

void
text_init(void)
{
	texture_init_from_file(&texture_font, 128, 128,
			       "assets/textures/font");

	for (int i = '!'; i <= '}'; i++) {
		int row = (i - '!') / 16;
		int col = (i - '!') % 16;
		struct sprite *s = malloc(sizeof (struct sprite));
		sprite_init(s, &texture_font, 8*col, 16*row, 8, 16);
		ascii_sprites[i] = s;
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

void
draw_text(struct scenegraph *scenegraph, char *str, float x, float y,
	  int alignment)
{
	char c;
	float pixel_size = 2.0/scenegraph->height;

	switch (alignment) {
	case TEXT_TOPLEFT:
		x += scale * 4.0 * pixel_size;
		y -= scale * 8.0 * pixel_size;
		break;
	case TEXT_CENTRE:
		x -= scale * 4.0 * pixel_size * (strlen(str) - 1);
		break;
	}

	while ((c = *str++)) {
		if (ascii_sprites[c] != NULL) {
			struct sprite *s = ascii_sprites[c];
			s->scale = scale;
			s->base_color = color;
			s->x = x;
			s->y = y;
			sprite_draw(scenegraph, s);
		}
		x += scale * 8.0 * pixel_size;
	}
}
