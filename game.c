#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "menu.h"
#include "input.h"

int game_selected_square, game_selected_move;

static enum {
	SELECT_PIECE,
	SELECT_MOVE
} cur_mode;

uint64_t
piece_moves(int i)
{
	uint64_t moves = 0;
	int y = i/8;
	int x = i%8;
	if (x > 0 && y < 7) {
		int i2 = 8*(y + 1) + x - 1;
		moves |= (uint64_t)1<<i2;
	}
	if (x < 7 && y < 7)
		moves |= (uint64_t)1<<(8*(y + 1) + x + 1);
	return moves;
}

static void
game_init(void)
{
	game_selected_square = 0;
	cur_mode = SELECT_PIECE;
	game_display_init();
}

static void
game_update(void)
{
}

static void
game_input_event(int button)
{
	if (button == INPUT_PAUSE) {
		menu.init();
		checkers_switch_state(&menu);
		return;
	}

	if (cur_mode == SELECT_PIECE) {
		switch (button) {
		case INPUT_ACCEPT:
			cur_mode = SELECT_MOVE;
			{
				uint64_t moves = piece_moves(game_selected_square);
				for (int i = 0; i < 64; i++) {
					if ((moves >> i) & 1) {
						game_selected_move = i;
						break;
					}
				}
			}
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

	if (cur_mode == SELECT_MOVE) {
		switch (button) {
		case INPUT_BACK:
			cur_mode = SELECT_PIECE;
			game_selected_move = -1;
			return;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			{
				uint64_t moves = piece_moves(game_selected_square);
				for (int i = game_selected_move + 1; i != game_selected_move; i = (i+1)%64) {
					if ((moves >> i) & 1) {
						game_selected_move = i;
						break;
					}
				}
			}
			break;
		}
	}
}

struct state game = {
	.load = game_display_load,
	.init = game_init,
	.update = game_update,
	.input_event = game_input_event
};
