#ifndef _CHECKERS_H_
#define _CHECKERS_H_

#ifdef __ANDROID__
#define TEXTURES_DIR	"textures/"
#else
#define TEXTURES_DIR	"assets/textures/"
#endif

#include "scenegraph.h"

struct state {
	void (*load)(void);
	void (*init)(void);
	void (*update)(void);
	void (*input_event)(int);
	struct scenegraph sg;
};

void checkers_init(void);
void checkers_update(void);
void checkers_input_event(int button);
void checkers_switch_state(struct state *new_state);

#ifdef __ANDROID__
#include <jni.h>

/* JNIEnv for GLSurfaceView's rendering thread */
extern JNIEnv *checkers_jnienv;
/* jeandre.checkers.Checkers instance */
extern jobject checkers_java;
#endif

#endif /* !_CHECKERS_H_ */
