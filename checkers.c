#include "checkers.h"
#include "game.h"
#include "main_menu.h"
#include "text.h"

static struct state *current_state = NULL;

void
checkers_init(void)
{
	text_init();
	game.load();
	game.init();
	main_menu.load();
	main_menu.init();
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

void
checkers_input_event(int button)
{
	current_state->input_event(button);
}
