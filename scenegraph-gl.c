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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "config.h"
#ifdef USE_GL_ES
#include <GLES/gl.h>
#define glFrustum glFrustumf
#else
#include <GL/gl.h>
#endif

#include "scenegraph.h"
#include "texture.h"

static int width, height;

void
sg_init(int w, int h)
{
	width = w;
	height = h;
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
}

void
sg_init_scenegraph(struct scenegraph *scenegraph)
{
	if (scenegraph->light0_enabled) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
	}
	scenegraph->width = width;
	scenegraph->height = height;
}

void
sg_render(struct scenegraph *scenegraph)
{
	GLfloat light_pos[] = {
		scenegraph->light0_x, scenegraph->light0_y,
		scenegraph->light0_z, 1
	};
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	if (scenegraph->cam3d_enabled) {
		float tan_fov_2 = tanf(scenegraph->fov/2.0 * M_PI/180.0);
		float aspect = (float)width/(float)height;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-0.1 * tan_fov_2 * aspect,
			  0.1 * tan_fov_2 * aspect,
			  -0.1 * tan_fov_2, 0.1 * tan_fov_2,
			  scenegraph->near_plane,
			  scenegraph->far_plane);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	glRotatef(-scenegraph->cam_dir_vert * 180/M_PI, 1, 0, 0);
	glRotatef(-scenegraph->cam_dir_horiz * 180/M_PI, 0, 1, 0);
	glTranslatef(-scenegraph->cam_x, -scenegraph->cam_y,
		     -scenegraph->cam_z);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	for (size_t i = 0; i < scenegraph->num_render; i++) {
		scenegraph->render[i](scenegraph);
	}
}

void
sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj)
{
	if (obj->flags & SG_OBJ_NOLIGHTDEPTH) {
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
	}
	if (obj->flags & SG_OBJ_TEXTURED) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, obj->texture->gl_tex);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	glColor4f((obj->color & 0xff)/255.0,
		  ((obj->color >> 8) & 0xff)/255.0,
		  ((obj->color >> 16) & 0xff)/255.0,
		  (obj->color >> 24)/255.0);
	glPushMatrix();
	glTranslatef(obj->x, obj->y, obj->z);
	if (obj->flags & SG_OBJ_TEXTURED) {
		int stride = 8*sizeof (float);
		glTexCoordPointer(2, GL_FLOAT, stride, obj->vertices);
		glNormalPointer(GL_FLOAT, stride, &obj->vertices[2]);
		glVertexPointer(3, GL_FLOAT, stride, &obj->vertices[5]);
	} else {
		int stride = 6*sizeof (float);
		glNormalPointer(GL_FLOAT, stride, obj->vertices);
		glVertexPointer(3, GL_FLOAT, stride, &obj->vertices[3]);
	}
	glDrawArrays(GL_TRIANGLES, 0, obj->num_vertices);
	glPopMatrix();
	if (obj->flags & SG_OBJ_TEXTURED) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
	if (obj->flags & SG_OBJ_NOLIGHTDEPTH) {
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
}
