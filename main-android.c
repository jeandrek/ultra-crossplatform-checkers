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
 * We could rather use NativeActivity/native_app_glue.
 */

#include <sys/stat.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GLES/gl.h>
#include <jni.h>

#include "checkers.h"
#include "scenegraph.h"
#include "game.h"
#include "game_display.h"
#include "text.h"
#include "select_file.h"

/* JNIEnv for GLSurfaceView's rendering thread */
JNIEnv *checkers_jnienv;
/* jeandre.checkers.Checkers instance */
jobject checkers_java;

char *save_file_dir;

/* Big lock */
static pthread_mutex_t checkers_mutex = PTHREAD_MUTEX_INITIALIZER;

static int initialized = 0;

void
enter_android_call(JNIEnv *env, jobject *checkers)
{
	pthread_mutex_lock(&checkers_mutex);
	checkers_jnienv = env;
	checkers_java = checkers;
}

void
leave_android_call(void)
{
	pthread_mutex_unlock(&checkers_mutex);
}

JNIEXPORT jboolean JNICALL
Java_jeandre_checkers_Checkers_initialized(JNIEnv *env, jobject obj)
{
	return initialized;
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_init(JNIEnv *env, jobject obj,
				    jint width, jint height,
				    jstring files_dir,
				    jboolean check_autosave)
{
	const char *files_dir_str;

	enter_android_call(env, obj);

	files_dir_str = (*env)->GetStringUTFChars(env, files_dir, NULL);
	save_file_dir = strdup(files_dir_str);
	(*env)->ReleaseStringUTFChars(env, files_dir, files_dir_str);

	if (check_autosave) {
		char path[128];
		struct stat s;
		snprintf(path, 128, "%s/autosave.checkers", save_file_dir);
		if (stat(path, &s) == 0)
			initial_filepath = path;
	}

	sg_init(width, height);
	checkers_init();
	text_scale_factor = 2.0;

	initialized = 1;

	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_recreateGlObjectsAndState(JNIEnv *env,
							 jobject obj,
							 jint width,
							 jint height)
{
	enter_android_call(env, obj);
	sg_init(width, height);
	text_destroy();
	game.unload();
	text_init();
	game.load();
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_resize(JNIEnv *env, jobject obj,
				      jint width, jint height)
{
	enter_android_call(env, obj);
	checkers_resize(width, height);
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_update(JNIEnv *env, jobject obj)
{
	enter_android_call(env, obj);
	/* input_handle(); */
	checkers_update();
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_inputEvent(JNIEnv *env, jobject obj, jint button)
{
	enter_android_call(env, obj);
	/* Alternatively, this could enqueue events and input_handle processes
	   them; but this is fine for now. */
	checkers_button_event(button);
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_mouseMoveEvent(JNIEnv *env, jobject obj,
					      jint x, jint y)
{
	enter_android_call(env, obj);
	checkers_mouse_move(x, y);
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_mouseUpEvent(JNIEnv *env, jobject obj,
					    jint x, jint y)
{
	enter_android_call(env, obj);
	checkers_mouse_up(x, y);
	leave_android_call();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_autosave(JNIEnv *env, jobject obj)
{
	char path[256];

	enter_android_call(env, obj);
	snprintf(path, 256, "%s/autosave.checkers", save_file_dir);
	if (game_can_save() && game_dirty)
		game_save(path);
	else if (game_type != NO_GAME && game_is_over())
		unlink(path);
	leave_android_call();
}
