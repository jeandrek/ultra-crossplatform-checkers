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

#if defined(USE_GL_ES)
#include <GLES/gl.h>
#define glFrustum glFrustumf
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
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
	glEnable(GL_STENCIL_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
}

void
sg_resize(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

void
sg_init_scenegraph(struct scenegraph *scenegraph)
{
	scenegraph->width = width;
	scenegraph->height = height;
}

void
sg_update(struct scenegraph *scenegraph)
{
	if (scenegraph->width != width || scenegraph->height != height) {
		scenegraph->width = width;
		scenegraph->height = height;
		if (scenegraph->resize != NULL)
			scenegraph->resize();
	}
}

void
render_piece_shadow_volume(struct scenegraph *scenegraph, float x, float y, float z)
{
	float light_pos[3] = {scenegraph->light0_x, scenegraph->light0_y,
			      scenegraph->light0_z};
	float circle[36];
	int silhouette_i = 0;
	float silhouette[72];

	for (int i = 0; i < 12; i++) {
		float theta = 2*M_PI*i/12;
		circle[3*i] = x + 0.1*cosf(theta);
		circle[3*i + 1] = y;
		circle[3*i + 2] = z + 0.1*sinf(theta);
	}

	float last_dp = NAN;
	for (int i = 0; i < 12; i++) {
		float normal[3];
		float light[3];
		normal[0] = 10*circle[3*i];
		normal[1] = 0;
		normal[2] = 10*circle[3*i + 2];
		light[0] = light_pos[0] - circle[3*i];
		light[1] = light_pos[1] - circle[3*i + 1];
		light[2] = light_pos[2] - circle[3*i + 2];
		float magnitude = sqrtf(light[0]*light[0] + light[1]*light[1] + light[2]*light[2]);
		light[0] /= magnitude;
		light[1] /= magnitude;
		light[2] /= magnitude;
		float dp = normal[0]*light[0] + normal[1]*light[1] + normal[2]*light[2];

#define add(off) do {							\
			silhouette[3*silhouette_i] = circle[3*i];	\
			silhouette[3*silhouette_i + 1] = circle[3*i + 1] + (off); \
			silhouette[3*silhouette_i + 2] = circle[3*i + 2]; \
			silhouette_i++;					\
		} while (0)

		if (last_dp < 0 && dp >= 0)
			add(0.02);
		else if (last_dp >= 0 && dp < 0)
			add(-0.03);

		if (dp < 0)
			add(0.02);
		else
			add(-0.03);
		last_dp = dp;
	}

	glBegin(GL_QUAD_STRIP);
	for (int j = 0; j <= silhouette_i; j++) {
		int i = j % silhouette_i;
		float light[3];
		light[0] = light_pos[0] - silhouette[3*i];
		light[1] = light_pos[1] - silhouette[3*i + 1];
		light[2] = light_pos[2] - silhouette[3*i + 2];
		float magnitude = sqrtf(light[0]*light[0] + light[1]*light[1] + light[2]*light[2]);
		light[0] /= magnitude;
		light[1] /= magnitude;
		light[2] /= magnitude;
		glVertex3f(silhouette[3*i] - 10*light[0],
			   silhouette[3*i + 1] - 10*light[1],
			   silhouette[3*i + 2] - 10*light[2]);
		glVertex3f(silhouette[3*i],
			   silhouette[3*i + 1],
			   silhouette[3*i + 2]);
	}
	glEnd();
}

void
sg_render(struct scenegraph *scenegraph)
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
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
	if (scenegraph->light0_enabled) {
		GLfloat light_pos[] = {
			scenegraph->light0_x, scenegraph->light0_y,
			scenegraph->light0_z, 1
			/* 0.707, 0.707, 0, 0 */
		};
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	}
	for (size_t i = 0; i < scenegraph->num_render; i++) {
		scenegraph->render[i](scenegraph);
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_ALWAYS, 0, 0xff);
	glStencilOp(GL_KEEP,  GL_KEEP, GL_INVERT);

	render_shadow_volumes(scenegraph);

	glFrontFace(GL_CW);
	render_shadow_volumes(scenegraph);
	glFrontFace(GL_CCW);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glDepthFunc(GL_EQUAL);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 0, 0xff);
	glDisable(GL_LIGHT0);
	for (size_t i = 0; i < scenegraph->num_render; i++) {
		scenegraph->render[i](scenegraph);
	}
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_EQUAL, 0, 0xff);
}

void
sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj)
{
	float color[] = {(obj->color & 0xff)/255.0,
			 ((obj->color >> 8) & 0xff)/255.0,
			 ((obj->color >> 16) & 0xff)/255.0,
			 (obj->color >> 24)/255.0};
	if (obj->flags & SG_OBJ_NOLIGHTDEPTH) {
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glColor4f(color[0], color[1], color[2], color[3]);
	} else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
	}
	if (obj->flags & SG_OBJ_SPECULAR) {
		float spec_color[] = {0.3, 0.3, 0.3, 1};
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color);
		glMaterialf(GL_FRONT, GL_SHININESS, 100);
	} else {
		float spec_color[] = {0, 0, 0, 1};
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color);
	}
	if (obj->flags & SG_OBJ_TEXTURED) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, obj->texture->gl_tex);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
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
	if (obj->flags & SG_OBJ_INDEXED)
		glDrawElements(GL_TRIANGLES, obj->num_indices,
			       GL_UNSIGNED_BYTE, obj->indices);
	else
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
