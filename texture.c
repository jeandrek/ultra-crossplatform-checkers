#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef __psp__
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "scenegraph.h"
#include "texture.h"

void
texture_init(struct texture *texture, int width, int height, void *pixels)
{
	texture->width = width;
	texture->height = height;
#ifdef __psp__
	texture->buffer = guGetStaticVramBuffer(width, height, GU_PSM_8888);
	texture->buffer = (void *)(0x4000000 + (uint32_t)texture->buffer);
	memcpy(texture->buffer, pixels, 4*width*height);
	/* sceGuCopyImage(GU_PSM_8888, 0, 0, width, height, width, */
	/* 	       pixels, 0, 0, width, texture->buffer); */
	sceGuTexSync();
	sceKernelDcacheWritebackInvalidateRange(texture->buffer,
						4*width*height);
#else
	glGenTextures(1, &texture->gl_tex);
	glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
}

void
texture_init_from_file(struct texture *texture, int width, int height,
		       char *path)
{
	size_t size = 4*width*height;
	char *buf = malloc(size);
	FILE *f = fopen(path, "rb");
	fread(buf, size, 1, f);
	fclose(f);
	texture_init(texture, width, height, buf);
	free(buf);
}
