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
#include "asset.h"
#include "scenegraph.h"

static GLuint lighting_enabled_location;
static GLuint texturing_enabled_location;
static GLuint texture_location;

GLuint sg_program;

static GLuint
compile_shader(GLenum type, const char *src, size_t len)
{
	GLuint shader = glCreateShader(type);
	GLint src_len = len;
	GLint status;

	glShaderSource(shader, 1, &src, &src_len);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		GLint log_size;
		char *log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		log = malloc(log_size);
		glGetShaderInfoLog(shader, log_size, NULL, log);
		fprintf(stderr, "checkers: %s shader compilation error\n%s\n",
			type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			log);
		free(log);
		exit(1);
	}

	return shader;
}

unsigned int
sg_shaders_compile_program(const char *vert_name, const char *frag_name)
{
	size_t vert_src_len, frag_src_len;
	char *vert_src = asset_read(vert_name, SHADER, &vert_src_len);
	char *frag_src = asset_read(frag_name, SHADER, &frag_src_len);
	GLuint program = glCreateProgram();
	GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_src, vert_src_len);
	GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src, frag_src_len);
	GLint status;

	asset_free(vert_src);
	asset_free(frag_src);

	glAttachShader(program, frag_shader);
	glAttachShader(program, vert_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		GLint log_size;
		char *log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
		log = malloc(log_size);
		glGetProgramInfoLog(program, log_size, NULL, log);
		fprintf(stderr, "checkers: linking error\n%s\n", log);
		free(log);
		exit(1);
	}

	return program;
}

void
sg_shaders_init(void)
{
	sg_program = sg_shaders_compile_program(SHADERS_DIR "vert",
						SHADERS_DIR "frag");

	lighting_enabled_location =
		glGetUniformLocation(sg_program, "lighting_enabled");
	texturing_enabled_location =
		glGetUniformLocation(sg_program, "texturing_enabled");
	texture_location = glGetUniformLocation(sg_program, "texture");

	glUseProgram(sg_program);

	glUniform1i(texture_location, 0);

}

int
sg_using_scenegraph_program(void)
{
	int p;
	glGetIntegerv(GL_CURRENT_PROGRAM, &p);
	return p == sg_program;
}

void
sg_shaders_set_lighting_enabled(int enabled)
{
	if (!sg_using_scenegraph_program()) return;
	glUniform1i(lighting_enabled_location, enabled);
}

void
sg_shaders_set_texturing_enabled(int enabled)
{
	if (!sg_using_scenegraph_program()) return;
	glUniform1i(texturing_enabled_location, enabled);
}
