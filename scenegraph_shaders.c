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

static const char *vert_src =
	"#version 140\n"
	"uniform int lighting_enabled;\n"
	//"varying vec4 pos_eye;\n"
	//"varying vec3 normal;\n"
	"varying vec2 texcoord;\n"
	"varying vec4 color;\n"
	"varying float diffuse_fac;\n"
	"void main()\n"
	"{\n"
	"  vec4 pos_eye;\n"
	"  vec3 normal;\n"
	"  pos_eye = gl_ModelViewMatrix * gl_Vertex;\n"
	"  normal = gl_NormalMatrix * gl_Normal;\n"
	"  texcoord = vec2(gl_MultiTexCoord0);\n"
	"  color = gl_Color;\n"
	"  vec3 light_dir = normalize(vec3(gl_LightSource[0].position - pos_eye));\n"
	"  diffuse_fac = clamp(dot(normal, light_dir), 0, 1);\n"
	"  if (lighting_enabled == 1) {\n"
	"    color = gl_FrontLightProduct[0].diffuse * vec4(vec3(diffuse_fac), 1);\n"
	"    color += gl_FrontLightModelProduct.sceneColor;\n"
	"    color = clamp(color, 0, 1);\n"
	"  }\n"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n";

static const char *frag_src =
	"#version 140\n"
	"uniform int texturing_enabled;\n"
	"uniform sampler2D texture;\n"
	//"varying vec4 pos_eye;\n"
	//"varying vec3 normal;\n"
	"varying vec2 texcoord;\n"
	"varying vec4 color;\n"
	"varying float diffuse_fac;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"  if (texturing_enabled == 1) {\n"
	"    gl_FragColor *= texture2D(texture, texcoord);\n"
	"  }\n"
	"}\n";

static GLuint lighting_enabled_location;
static GLuint texturing_enabled_location;
static GLuint texture_location;

static GLuint
compile_shader(GLenum type, const char *src)
{
	GLuint shader = glCreateShader(type);
	GLint len = strlen(src);
	GLint status;

	glShaderSource(shader, 1, &src, &len);
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

void
sg_shaders_init(void)
{
	GLuint program = glCreateProgram();
	GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_src);
	GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src);
	GLint status;

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

	lighting_enabled_location =
		glGetUniformLocation(program, "lighting_enabled");
	texturing_enabled_location =
		glGetUniformLocation(program, "texturing_enabled");
	texture_location = glGetUniformLocation(program, "texture");

	glUniform1i(texture_location, 0);

	glUseProgram(program);
}

void
sg_shaders_set_lighting_enabled(int enabled)
{
	glUniform1i(lighting_enabled_location, enabled);
}

void
sg_shaders_set_texturing_enabled(int enabled)
{
	glUniform1i(texturing_enabled_location, enabled);
}
