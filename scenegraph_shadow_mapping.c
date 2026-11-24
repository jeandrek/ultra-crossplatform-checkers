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

/*
 * Shadow mapping
 * Only one shadow map is allowed in a scene.
 */

#include <math.h>

#if defined(USE_GL_ES)
#include <GLES/gl.h>		/* XXX */
#define glFrustum glFrustumf
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "checkers.h"
#include "scenegraph.h"

#define SHADOW_MAP_SIZE		1024

static GLuint shadow_map_program;

static GLuint shadow_fbo;
static GLuint shadow_map_tex;

static GLuint shadows_enabled_location;
static GLuint shadow_map_location;
static GLuint view_location;
static GLuint shadow_view_location;
static GLuint shadow_proj_location;

static const char dummy_pixels[SHADOW_MAP_SIZE*SHADOW_MAP_SIZE*4] = {0};

void
sg_shadow_mapping_init(void)
{
	glGenFramebuffers(1, &shadow_fbo);
	glGenTextures(1, &shadow_map_tex);

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &shadow_map_tex);
	glBindTexture(GL_TEXTURE_2D, shadow_map_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		     SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0,
		     GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, dummy_pixels);
	glActiveTexture(GL_TEXTURE0);

	shadows_enabled_location = glGetUniformLocation(sg_program, "shadows_enabled");
	shadow_map_location = glGetUniformLocation(sg_program, "shadow_map");
	view_location = glGetUniformLocation(sg_program, "view");
	shadow_view_location = glGetUniformLocation(sg_program, "shadow_view");
	shadow_proj_location = glGetUniformLocation(sg_program, "shadow_proj");

	glUniform1i(shadows_enabled_location, 1);
	glUniform1i(shadow_map_location, 1);

	shadow_map_program =
		sg_shaders_compile_program(SHADERS_DIR "vert-shadow-map",
					   SHADERS_DIR "frag-shadow-map");
}

void
sg_shadow_mapping_render_map(struct scenegraph *scenegraph)
{
	float view[16], proj[16];
	float tan_fov_2 = tanf(50 * M_PI/180);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			       shadow_map_tex, 0);

	glUseProgram(shadow_map_program);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-tan_fov_2, tan_fov_2, -tan_fov_2, tan_fov_2, 1, 4);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(90, 1, 0, 0);
	glTranslatef(-scenegraph->light0_x,
		     -scenegraph->light0_y,
		     -scenegraph->light0_z);

	glGetFloatv(GL_MODELVIEW_MATRIX, view);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 8);

	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glClear(GL_DEPTH_BUFFER_BIT);
	sg_render_for_shadow(scenegraph);
	glViewport(0, 0, scenegraph->width, scenegraph->height);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glUseProgram(sg_program);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glUniformMatrix4fv(shadow_view_location, 1, GL_FALSE, view);
	glUniformMatrix4fv(shadow_proj_location, 1, GL_FALSE, proj);
}

void
sg_shadow_mapping_begin_render(void)
{
	float view[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, view);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, view);
}

void
sg_shadow_mapping_set_shadows_enabled(int enabled)
{
	if (!sg_using_scenegraph_program()) return;
	glUniform1i(shadows_enabled_location, enabled);
}
