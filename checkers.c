#include "checkers.h"
#include "game.h"
#include "menu.h"

static struct state *current_state = NULL;

void
checkers_init(void)
{
	game.load();
	game.init();
	menu.load();
	menu.init();
	current_state = &game;
}

void
checkers_switch_state(struct state *new_state)
{
	current_state = new_state;
}

void
checkers_update(void)
{
	current_state->update();
	sg_render(&current_state->sg);
}
