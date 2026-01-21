/*
 * We could rather use NativeActivity/native_app_glue, but the (currently
 * rather poor) Android UI uses several ButtonViews for input, rather than
 * supporting proper touch input.
 */

#include <jni.h>
#include <GLES/gl.h>

#include "checkers.h"
#include "scenegraph.h"

/* JNIEnv for GLSurfaceView's rendering thread */
JNIEnv *checkers_jnienv;
/* jeandre.checkers.Checkers class */
jclass checkers_java;

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_init(JNIEnv *env, jobject obj,
				    jint width, jint height)
{
	checkers_jnienv = env;
	checkers_java = obj;
	sg_init(width, height);
	checkers_init();
	//input_init();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_update(JNIEnv *env, jobject obj)
{
	//input_handle();
	checkers_update();
}

JNIEXPORT void JNICALL
Java_jeandre_checkers_Checkers_inputEvent(JNIEnv *env, jobject obj, jint button)
{
	checkers_input_event(button); /* XXX */
}
