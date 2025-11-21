#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "menu.h"
#include "input.h"

int game_selected_square = 0;

static void
game_init(void)
{
	game_display_init();
}

static void
game_update(void)
{
	switch (input_read()) {
	case INPUT_PAUSE:
		menu.init();
		checkers_switch_state(&menu);
		break;
	case INPUT_UP:
		game_selected_square = (game_selected_square + 8) % 64;
		break;
	case INPUT_DOWN:
		game_selected_square -= 8;
		if (game_selected_square < 0) game_selected_square += 64;
		break;
	case INPUT_LEFT:
		game_selected_square -= 1;
		if (game_selected_square < 0) game_selected_square = 63;
		break;
	case INPUT_RIGHT:
		game_selected_square = (game_selected_square + 1) % 64;
		break;
	}
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update
};
