#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "menu.h"
#include "input.h"

static void
game_init(void)
{
	game_display_init();
}

static void
game_update(void)
{
	if (input_read() == 4) {
		menu.init();
		checkers_switch_state(&menu);
	}
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update
};
