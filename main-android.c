/*
 * We could rather use NativeActivity/native_app_glue, but the (currently
 * rather poor) Android UI uses several ButtonViews for input, rather than
 * supporting proper touch input.
 */

#include <jni.h>
#include <threads.h>
#include <GLES/gl.h>

#include "checkers.h"
#include "scenegraph.h"

/* JNIEnv for GLSurfaceView's rendering thread */
JNIEnv *checkers_jnienv;
/* jeandre.checkers.Checkers class */
jclass checkers_java;

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
	   them.  This is fine for now but it probably needs to be done that
	   way with networking/computer opponent. */
	checkers_input_event(button);
	mtx_unlock(&checkers_mutex);
}
