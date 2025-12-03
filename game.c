#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "game_interaction.h"

enum mode cur_mode;

static void
game_init(void)
{
	game_interaction_init();
	game_display_init();
}

static void
game_update(void)
{
	if (cur_mode == ANIM_ROTATE_BOARD)
		game_anim();
}

void
game_start_anim_rotate(void)
{
	cur_mode = ANIM_ROTATE_BOARD;
}

void
game_anim_rotate_finished(void)
{
	cur_mode = SELECT_PIECE;
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update,
	.input_event = game_input_event
};
