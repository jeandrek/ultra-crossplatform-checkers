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

#include "config.h"
#if defined(__psp__)
#include <pspgu.h>
#include <pspgum.h>
#elif defined(USE_GL_ES)
#include <GLES/gl.h>
#define glOrtho glOrthof
#else
#include <GL/gl.h>
#endif

#include "sprite.h"
#include "scenegraph.h"
#include "texture.h"

#ifndef __psp__
static float sprite_verts[] = {
	-1, -1,
	1, -1,
	1, 1,
	-1, 1
};

static uint16_t sprite_indices[] = {0, 1, 2, 0, 2, 3};
#endif

void
sprite_init(struct sprite *sprite, struct texture *texture,
	    int tex_left, int tex_top, int width, int height)
{
	sprite->texture = texture;
	sprite->tex_left = tex_left;
	sprite->tex_top = tex_top;
	sprite->width = width;
	sprite->height = height;
	sprite->x = 0;
	sprite->y = 0;
	sprite->scale = 1.0;
	sprite->base_color = ~0;

#ifndef __psp__
	if (texture == NULL) return;
	float ss_w = sprite->texture->width;
	float ss_h = sprite->texture->height;

	sprite->tex_coord[0] = sprite->tex_left/ss_w;
	sprite->tex_coord[1] = (sprite->tex_top + sprite->height)/ss_h;
	sprite->tex_coord[2] = (sprite->tex_left + sprite->width)/ss_w;
	sprite->tex_coord[3] = (sprite->tex_top + sprite->height)/ss_h;
	sprite->tex_coord[4] = (sprite->tex_left + sprite->width)/ss_w;
	sprite->tex_coord[5] = sprite->tex_top/ss_h;
	sprite->tex_coord[6] = sprite->tex_left/ss_w;
	sprite->tex_coord[7] = sprite->tex_top/ss_h;
#endif
}

void
sprite_draw(struct scenegraph *scenegraph, struct sprite *sprite)
{
#ifdef __psp__
	float *sprite_verts = sceGuGetMemory(10 * sizeof (float));

	sprite_verts[0] = sprite->tex_left;
	sprite_verts[1] = sprite->tex_top;
	sprite_verts[2] = 240 + 136 * sprite->x - sprite->width / 2.0;
	sprite_verts[3] = 136 - 136 * sprite->y - sprite->height / 2.0;

	sprite_verts[5] = sprite->tex_left + sprite->width;
	sprite_verts[6] = sprite->tex_top + sprite->height;
	sprite_verts[7] = 240 + 136 * sprite->x + sprite->width / 2.0;
	sprite_verts[8] = 136 - 136 * sprite->y + sprite->height / 2.0;

	sceGuDisable(GU_DEPTH_TEST);
	if (sprite->texture != NULL) {
		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexMode(GU_PSM_8888, 0, 0, 0);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuTexImage(0, sprite->texture->width,
			      sprite->texture->height,
			      sprite->texture->width, sprite->texture->buffer);
	}
	sceGuColor(sprite->base_color);
	sceGuDrawArray(GU_SPRITES,
		       GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_TRANSFORM_2D,
		       2, 0, sprite_verts);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuEnable(GU_DEPTH_TEST);
#else
	float aspect = (float)scenegraph->width / (float)scenegraph->height;

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

	if (sprite->texture != NULL) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, sprite->texture->gl_tex);
	}
	glColor4f((sprite->base_color & 0xff)/255.0,
		  ((sprite->base_color >> 8) & 0xff)/255.0,
		  ((sprite->base_color >> 16) & 0xff)/255.0,
		  (sprite->base_color >> 24)/255.0);

	glVertexPointer(2, GL_FLOAT, 0, sprite_verts);
	glTexCoordPointer(2, GL_FLOAT, 0, sprite->tex_coord);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, sprite_indices);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
#endif
}
