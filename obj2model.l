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

%{
#include <stdint.h>
#include <stdio.h>

#define MAX_VERTS	256
#define MAX_INDICES	32768

float positions[3*MAX_VERTS];
float texcoords[2*MAX_VERTS];
float normals[3*MAX_VERTS];

int positions_i = 0;
int texcoords_i = 0;
int normals_i = 0;

int cur_position = 0;
int cur_texcoord = 0;
int cur_normal = 0;

uint32_t num_verts = 0;

int textured = 0;

void add_vertex(void);
void too_many(void);
%}

%s POSITION TEXCOORD NORMAL FACE
%x POST_POSITION POST_TEXCOORD
%x PRE_NORMAL PRE_TEXCOORD

COORD	-?[0-9]+("."[0-9]+)?
INDEX	[0-9]+

%%
#.*\n
o" ".+\n
s" ".+\n
mtllib" ".+\n
usemtl" ".+\n
v			BEGIN POSITION;
<POSITION>{COORD}	{
	if (positions_i == 3*MAX_VERTS) too_many();
	positions[positions_i++] = atof(yytext);
	if (positions_i % 3 == 0) BEGIN 0;
	}
vt			BEGIN TEXCOORD;
<TEXCOORD>{COORD}	{
	if (texcoords_i == 2*MAX_VERTS) too_many();
	texcoords[texcoords_i++] = atof(yytext);
	if (texcoords_i % 2 == 0) BEGIN 0;
	}
vn			BEGIN NORMAL;
<NORMAL>{COORD}		{
	if (normals_i == 3*MAX_VERTS) too_many();
	normals[normals_i++] = atof(yytext);
	if (normals_i % 3 == 0) BEGIN 0;
	}
f			BEGIN FACE;
<FACE>{INDEX}		{ cur_position = atoi(yytext); BEGIN POST_POSITION; }
<POST_POSITION>"/"	BEGIN PRE_TEXCOORD;
<PRE_TEXCOORD>{INDEX}	{
	cur_texcoord = atoi(yytext);
	textured = 1;
	BEGIN POST_TEXCOORD;
	}
<PRE_TEXCOORD>"/"	{ cur_texcoord = 0; BEGIN PRE_NORMAL; }
<POST_TEXCOORD>"/"	BEGIN PRE_NORMAL;
<PRE_NORMAL>{INDEX}	{ cur_normal = atoi(yytext); add_vertex(); BEGIN FACE; }
[ \t\n]			;
.			{
	fprintf(stderr, "error: unexpected character: %s\n", yytext);
	exit(1);
	}
%%
struct {
	int	position_idx;
	int	texcoord_idx;
	int	normal_idx;
} vertices[MAX_VERTS];

uint8_t indices[MAX_INDICES];
uint32_t indices_i;

void
add_vertex(void)
{
	int i;

	for (i = 0; i < num_verts; i++) {
		if (vertices[i].position_idx == cur_position &&
		    vertices[i].texcoord_idx == cur_texcoord &&
		    vertices[i].normal_idx == cur_normal) {
			indices[indices_i++] = i;
			return;
		}
	}
	if (num_verts == MAX_VERTS)
		too_many();
	num_verts++;
	vertices[i].position_idx = cur_position;
	vertices[i].texcoord_idx = cur_texcoord;
	vertices[i].normal_idx = cur_normal;
	indices[indices_i++] = i;
}

void
too_many(void)
{
	fprintf(stderr, "error: more than 256 verts\n");
	exit(1);
}

void
output(void)
{
	uint32_t format = 0;
	uint32_t padding = 0;
	if (textured) format = 1;
	fwrite(&format, sizeof (format), 1, stdout);
	fwrite(&num_verts, sizeof (num_verts), 1, stdout);
	fwrite(&indices_i, sizeof (format), 1, stdout);
	fwrite(&padding, sizeof (padding), 1, stdout);
	for (int i = 0; i < num_verts; i++) {
		int j = 2*(vertices[i].texcoord_idx - 1);
		int k = 3*(vertices[i].normal_idx - 1);
		int l = 3*(vertices[i].position_idx - 1);
		if (textured) {
			if (j >= 0) {
				fwrite(&texcoords[j], sizeof (float), 2,
				       stdout);
			} else {
				float zero_texcoords[] = {0, 0};
				fwrite(zero_texcoords, sizeof (float), 2,
				       stdout);
			}
		}
		fwrite(&normals[k], sizeof (float), 3, stdout);
		fwrite(&positions[l], sizeof (float), 3, stdout);
	}
	fwrite(indices, 1, indices_i, stdout);
}

int
main(void)
{
	yylex();
	output();
}
