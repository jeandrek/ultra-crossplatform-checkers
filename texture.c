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

#ifdef __unix__
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <jni.h>
#endif

#if defined(__psp__)
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#elif defined(USE_GL_ES)
#include <GLES/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#include <GL/gl.h>
#endif

#include "checkers.h"
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
}

#ifdef __ANDROID__
static void
texture_data_from_asset(char *path, char *buf)
{
	JNIEnv *env = checkers_jnienv;
	jclass cls = (*env)->FindClass(env, "jeandre/checkers/Checkers");
	jmethodID mid = (*env)->GetMethodID(env, cls, "textureDataFromAsset",
					    "(Ljava/lang/String;)[B");
	jstring path_obj = (*env)->NewStringUTF(env, path);
	jbyteArray data = (*env)->CallObjectMethod(env, checkers_java,
						   mid, path_obj);

	size_t len = (*env)->GetArrayLength(env, data);
	int8_t *temp = (*env)->GetByteArrayElements(env, data, NULL);
	memcpy(buf, temp, len);
	(*env)->ReleaseByteArrayElements(env, data, temp, 0);
	(*env)->DeleteLocalRef(env, data);
}
#endif

void
texture_init_from_file(struct texture *texture, int width, int height,
		       char *name)
{
	char path[128];
#if defined(__APPLE__)
	CFBundleRef bundle;
	CFURLRef url;

	bundle = CFBundleGetMainBundle();
	url = CFBundleCopyResourceURL(bundle,
				      CFStringCreateWithCString(NULL, name, 0),
				      NULL, CFSTR("textures"));
	if (url == NULL)
		exit(1);
	CFURLGetFileSystemRepresentation(url, 1, path, 128);
	CFRelease(url);
#elif defined(__unix__)
	struct stat s;

	snprintf(path, 128, DATADIR "/%s", name);
	if (stat(path, &s) != 0)
		strcpy(path, name);
#elif defined(_WIN32)
	int i;

	GetModuleFileName(NULL, path, 128);
	for (i = strlen(path) - 1; path[i] != '\\'; i--)
		;
	snprintf(path + i, 128 - i, "\\%s", name);
#elif defined(__psp__)
	strcpy(path, name);
#endif

	size_t size = 4*width*height;
	char *buf = malloc(size);
#ifdef __ANDROID__
	texture_data_from_asset(path, buf);
#else
	FILE *f = fopen(path, "rb");

	if (f == NULL) {
#if defined(__unix__) || defined(__APPLE__)
		perror("checkers: cannot open file");
		fprintf(stderr,
			"Make sure assets are in " DATADIR
			" or your working directory.\n");
#endif
		exit(1);
	}
	fread(buf, size, 1, f);
	fclose(f);
#endif
	texture_init(texture, width, height, buf);
	free(buf);
}
