#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "game_interaction.h"

static void
game_init(void)
{
	game_interaction_init();
	game_display_init();
}

static void
game_update(void)
{
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update,
	.input_event = game_input_event
};
