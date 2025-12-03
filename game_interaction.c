#include "checkers.h"
#include "game.h"
#include "game_interaction.h"
#include "game_checkers.h"
#include "menu.h"
#include "input.h"

int game_sel_square;
int game_sel_piece_moves_len;
int game_sel_piece_moves[MAX_MOVES];
int game_sel_move_idx;
static enum {
	SELECT_PIECE,
	SELECT_MOVE
} cur_mode;

void
game_interaction_init(void)
{
	game_sel_square = 0;
	game_sel_move_idx = 0;
	game_sel_piece_moves_len = piece_moves(game_sel_piece_moves, game_sel_square);
	cur_mode = SELECT_PIECE;
}

static void
move_sel_square(int num)
{
	if (game_sel_square + num < 0)
		game_sel_square = 0;
	else if (game_sel_square + num > 63)
		game_sel_square = 63;
	else
		game_sel_square += num;
	game_sel_piece_moves_len = piece_moves(game_sel_piece_moves, game_sel_square);
}

static void
move_piece(void)
{
	board[0] ^= (uint64_t)1<<(uint64_t)game_sel_square;
	game_sel_square = game_sel_piece_moves[game_sel_move_idx];
	board[0] |= (uint64_t)1<<(uint64_t)game_sel_square;
	game_sel_move_idx = 0;
	cur_mode = SELECT_PIECE;
	game_sel_piece_moves_len = piece_moves(game_sel_piece_moves, game_sel_square);
}

void
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
			break;
		case INPUT_UP:
			move_sel_square(8);
			break;
		case INPUT_DOWN:
			move_sel_square(-8);
			break;
		case INPUT_LEFT:
			move_sel_square(-1);
			break;
		case INPUT_RIGHT:
			move_sel_square(1);
			break;
		}
	} else if (cur_mode == SELECT_MOVE) {
		switch (button) {
		case INPUT_BACK:
			cur_mode = SELECT_PIECE;
			game_sel_move_idx = 0;
			break;
		case INPUT_ACCEPT:
			move_piece();
			break;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			game_sel_move_idx = (game_sel_move_idx + 1) % game_sel_piece_moves_len;
			break;
		}
	}
}
