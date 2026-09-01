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

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__ANDROID__)
#include <jni.h>
#elif defined(__unix__)
#include <sys/stat.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "checkers.h"
#include "asset.h"

#ifndef __ANDROID__
static FILE *
asset_fopen(const char *name, int type)
{
#if defined(__APPLE__)
	CFBundleRef bundle;
	CFURLRef url;
	char path[128];

	bundle = CFBundleGetMainBundle();
	url = CFBundleCopyResourceURL(bundle,
				      CFStringCreateWithCString(NULL, name, 0),
				      NULL,
				      type == TEXTURE ? CFSTR("textures")
				      : CFSTR("models"));
	if (url == NULL)
		exit(1);
	CFURLGetFileSystemRepresentation(url, 1, (uint8_t *)path, 128);
	CFRelease(url);
#elif defined(__unix__) && !defined(__ANDROID__)
	char path[128];
	struct stat s;

	snprintf(path, 128, DATADIR "/%s", name);
	if (stat(path, &s) != 0)
		strcpy(path, name);
#elif defined(_WIN32)
	char path[128];
	int i;

	GetModuleFileName(NULL, path, 128);
	for (i = strlen(path) - 1; path[i] != '\\'; i--)
		;
	snprintf(path + i, 128 - i, "\\%s", name);
#elif defined(__psp__)
	char *path = name;
#endif
	FILE *f = fopen(path, "rb");

	if (f == NULL) {
#if defined(__unix__)
		perror("checkers: cannot open file");
		fprintf(stderr,
			"Make sure assets are in " DATADIR
			" or your working directory.\n");
#endif
		exit(1);
	}
	return f;
}
#endif

void *
asset_read(const char *name, int type, size_t *sizep)
{
	size_t size;
	char *buf;
#ifdef __ANDROID__
	JNIEnv *env = checkers_jnienv;
	jclass cls = (*env)->FindClass(env, "jeandre/checkers/Checkers");
	jmethodID mid = (*env)->GetMethodID(env, cls, "dataFromAsset",
					    "(Ljava/lang/String;)[B");
	jstring name_obj = (*env)->NewStringUTF(env, name);
	jbyteArray data = (*env)->CallObjectMethod(env, checkers_java,
						   mid, name_obj);

	int8_t *temp = (*env)->GetByteArrayElements(env, data, NULL);
	size = (*env)->GetArrayLength(env, data);
	buf = malloc(size);
	memcpy(buf, temp, size);
	(*env)->ReleaseByteArrayElements(env, data, temp, 0);
	(*env)->DeleteLocalRef(env, data);
#else
	FILE *f = asset_fopen(name, type);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
#ifdef __psp__
	buf = aligned_alloc(16, size);
#else
	buf = malloc(size);
#endif
	fread(buf, size, 1, f);
	fclose(f);
#endif
	if (sizep) *sizep = size;
	return buf;
}

void
asset_free(void *data)
{
	free(data);
}
