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
 * We could rather use NativeActivity/native_app_glue, but the (currently
 * rather poor) Android UI uses several Buttons for input, rather than
 * supporting proper touch input.
 */

#include <jni.h>
#include <threads.h>
#include <GLES/gl.h>

#include "checkers.h"
#include "scenegraph.h"

/* JNIEnv for GLSurfaceView's rendering thread */
JNIEnv *checkers_jnienv;
/* jeandre.checkers.Checkers instance */
jobject checkers_java;

static mtx_t checkers_mutex;	/* Big lock */

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_init(JNIEnv *env, jobject obj,
				    jint width, jint height)
{
	checkers_jnienv = env;
	checkers_java = obj;

	mtx_init(&checkers_mutex, mtx_plain);

	sg_init(width, height);
	checkers_init();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_update(JNIEnv *env, jobject obj)
{
	/* input_handle(); */
	mtx_lock(&checkers_mutex);
	checkers_update();
	mtx_unlock(&checkers_mutex);
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_inputEvent(JNIEnv *env, jobject obj, jint button)
{
	mtx_lock(&checkers_mutex);
	/* Alternatively, this could enqueue events and input_handle processes
	   them; but this is fine for now. */
	checkers_button_event(button);
	mtx_unlock(&checkers_mutex);
}
