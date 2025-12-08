#include "scenegraph.h"
#include "texture.h"

#ifdef __psp__
#include <pspgu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

static float __attribute__((aligned(16))) sprite_verts[] = {
	0, 0,
	-0.275735, 0.0919118, 0,
	0, 25,
	-0.275735, -0.0919118, 0,
	75, 0,
	0.275375, 0.0919118, 0,
	75, 0,
	0.275375, 0.0919118, 0,
	0, 25,
	-0.275375, -0.0919118, 0,
	75, 25,
	0.275375, -0.0919118, 0
};

void
draw_sprite(int left, int top, int width, int height, int centre_x, int centre_y, float scale,
	    struct texture *tex)
{
#ifdef __psp__
	sprite_verts[0] = top;
	sprite_verts[1] = left;
	sprite_verts[2] = ;
	sprite_verts[5] = top;
	sprite_verts[6] = left + width;
#else
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->gl_tex);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(left, top);
	glVertex2f(centre_x - scale*width/450, centre_y - scale*height/450);
	glTexCoord2f(left + width, top + height);
	glVertex2f(centre_x + scale*width/450, centre_y + scale*height/450);
	glTexCoord2f(left, top + height);
	glVertex2f(centre_x - scale*width/450, centre_y + scale*height/450);
	glEnd();
#endif
}
