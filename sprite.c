#include "sprite.h"
#include "scenegraph.h"
#include "texture.h"

#ifdef __psp__
#include <pspgu.h>
#include <pspgum.h>

static float __attribute__((aligned(16))) sprite_verts[30];
#else
#include <GL/gl.h>
#include <GL/glu.h>

static float sprite_verts[] = {
	-1, -1,
	1, -1,
	1, 1,
	-1, 1
};

static int sprite_indices[] = {0, 1, 2, 0, 2, 3};

#endif

void
sprite_draw(struct scenegraph *scenegraph, struct sprite *sprite)
{
#ifdef __psp__
	sprite_verts[0] = sprite->tex_left;
	sprite_verts[1] = sprite->tex_top;
	sprite_verts[2] = 240 + 240 * sprite->x - sprite->width / 2.0;
	sprite_verts[3] = 136 - 136 * sprite->y - sprite->height / 2.0;

	sprite_verts[5] = sprite->tex_left + sprite->width;
	sprite_verts[6] = sprite->tex_top;
	sprite_verts[7] = 240 + 240 * sprite->x + sprite->width / 2.0;
	sprite_verts[8] = 136 - 136 * sprite->y - sprite->height / 2.0;

	sprite_verts[10] = sprite->tex_left + sprite->width;
	sprite_verts[11] = sprite->tex_top + sprite->height;
	sprite_verts[12] = 240 + 240 * sprite->x + sprite->width / 2.0;
	sprite_verts[13] = 136 - 136 * sprite->y + sprite->height / 2.0;

	sprite_verts[15] = sprite->tex_left;
	sprite_verts[16] = sprite->tex_top;
	sprite_verts[17] = 240 + 240 * sprite->x - sprite->width / 2.0;
	sprite_verts[18] = 136 - 136 * sprite->y - sprite->height / 2.0;

	sprite_verts[20] = sprite->tex_left + sprite->width;
	sprite_verts[21] = sprite->tex_top + sprite->height;
	sprite_verts[22] = 240 + 240 * sprite->x + sprite->width / 2.0;
	sprite_verts[23] = 136 - 136 * sprite->y + sprite->height / 2.0;

	sprite_verts[25] = sprite->tex_left;
	sprite_verts[26] = sprite->tex_top + sprite->height;
	sprite_verts[27] = 240 + 240 * sprite->x - sprite->width / 2.0;
	sprite_verts[28] = 136 - 136 * sprite->y + sprite->height / 2.0;

	sceGuDisable(GU_DEPTH_TEST);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
	sceGuTexImage(0, sprite->texture->width, sprite->texture->height,
		      sprite->texture->width, sprite->texture->buffer);
	sceGuColor(0xffffffff);
	sceGumDrawArray(GU_TRIANGLES,
			GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_TRANSFORM_2D,
			6, 0, sprite_verts);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuEnable(GU_DEPTH_TEST);
#else
	float aspect = (float)scenegraph->width / (float)scenegraph->height;
	float ss_w = sprite->texture->width;
	float ss_h = sprite->texture->height;
	float sprite_tex_coord[8];

	sprite_tex_coord[0] = sprite->tex_left/ss_w;
	sprite_tex_coord[1] = (sprite->tex_top + sprite->height)/ss_h;
	sprite_tex_coord[2] = (sprite->tex_left + sprite->width)/ss_w;
	sprite_tex_coord[3] = (sprite->tex_top + sprite->height)/ss_h;
	sprite_tex_coord[4] = (sprite->tex_left + sprite->width)/ss_w;
	sprite_tex_coord[5] = sprite->tex_top/ss_h;
	sprite_tex_coord[6] = sprite->tex_left/ss_w;
	sprite_tex_coord[7] = sprite->tex_top/ss_h;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-aspect, aspect, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(sprite->x, sprite->y, 0);
	glScalef(sprite->scale * sprite->width/scenegraph->height,
		 sprite->scale * sprite->height/scenegraph->height, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sprite->texture->gl_tex);
	glColor3f(1, 1, 1);

	glVertexPointer(2, GL_FLOAT, 0, sprite_verts);
	glTexCoordPointer(2, GL_FLOAT, 0, sprite_tex_coord);
 	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, sprite_indices);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
#endif
}
