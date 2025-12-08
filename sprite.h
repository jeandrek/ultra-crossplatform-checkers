#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>

#include "scenegraph.h"

struct sprite {
	struct texture	*texture;
	int		tex_left;
	int		tex_top;
	int		width;
	int		height;
	float		x, y;
	float		scale;
	uint32_t	base_color;
#ifndef __psp__
	float		tex_coord[8];
#endif
};

void sprite_init(struct sprite *sprite, struct texture *texture,
		 int tex_left, int tex_top, int width, int height);
void sprite_draw(struct scenegraph *scenegraph, struct sprite *sprite);

#endif /* _SPRITE_H_ */
