#include "checkers.h"
#include "game.h"
#include "game_display.h"
#include "menu.h"
#include "input.h"

int game_sel_square, game_sel_move;
uint64_t game_sel_piece_moves;

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
	if (x > 0 && y < 7)
		moves |= (uint64_t)1<<(8*(y + 1) + x - 1);
	if (x < 7 && y < 7)
		moves |= (uint64_t)1<<(8*(y + 1) + x + 1);
	return moves;
}

static void
game_init(void)
{
	game_sel_square = 0;
	game_sel_move = -1;
	game_sel_piece_moves = piece_moves(game_sel_square);
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
		int old_square = game_sel_square;

		switch (button) {
		case INPUT_ACCEPT:
			cur_mode = SELECT_MOVE;
			for (int i = 0; i < 64; i++) {
				if ((game_sel_piece_moves >> i) & 1) {
					game_sel_move = i;
					break;
				}
			}
			break;
		case INPUT_UP:
			game_sel_square = (game_sel_square + 8) % 64;
			break;
		case INPUT_DOWN:
			game_sel_square -= 8;
			if (game_sel_square < 0) game_sel_square += 64;
			break;
		case INPUT_LEFT:
			game_sel_square -= 1;
			if (game_sel_square < 0) game_sel_square = 63;
			break;
		case INPUT_RIGHT:
			game_sel_square = (game_sel_square + 1) % 64;
			break;
		}

		if (game_sel_square != old_square)
			game_sel_piece_moves = piece_moves(game_sel_square);
	}

	if (cur_mode == SELECT_MOVE) {
		switch (button) {
		case INPUT_BACK:
			cur_mode = SELECT_PIECE;
			game_sel_move = -1;
			return;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			for (int i = game_sel_move + 1; i != game_sel_move; i = (i+1)%64) {
				if ((game_sel_piece_moves >> i) & 1) {
					game_sel_move = i;
					break;
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
