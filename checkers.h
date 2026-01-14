#ifndef _CHECKERS_H_
#define _CHECKERS_H_

#if defined(macintosh)
#define TEXTURES_DIR	":assets:textures:"
#elif defined(__APPLE__)
#define TEXTURES_DIR	""
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

#endif /* !_CHECKERS_H_ */
