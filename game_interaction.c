#include "checkers.h"
#include "game.h"
#include "game_interaction.h"
#include "game_checkers.h"
#include "menu.h"
#include "input.h"

int sel_square;
int sel_piece_moves_len;
int sel_piece_moves[MAX_MOVES];
int sel_move_idx;
static enum {
	SELECT_PIECE,
	SELECT_MOVE
} cur_mode;

void
game_interaction_init(void)
{
	sel_square = 0;
	sel_move_idx = 0;
	sel_piece_moves_len = piece_moves(sel_piece_moves, sel_square);
	cur_mode = SELECT_PIECE;
}

static void
move_sel_square(int num)
{
	if (sel_square + num < 0)
		sel_square = 0;
	else if (sel_square + num > 63)
		sel_square = 63;
	else
		sel_square += num;
	sel_piece_moves_len = piece_moves(sel_piece_moves, sel_square);
}

static void
move_piece(void)
{
	board[0] ^= (uint64_t)1<<(uint64_t)sel_square;
	sel_square = sel_piece_moves[sel_move_idx];
	board[0] |= (uint64_t)1<<(uint64_t)sel_square;
	sel_move_idx = 0;
	cur_mode = SELECT_PIECE;
	sel_piece_moves_len = piece_moves(sel_piece_moves, sel_square);
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
			sel_move_idx = 0;
			break;
		case INPUT_ACCEPT:
			move_piece();
			break;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			sel_move_idx = (sel_move_idx + 1) % sel_piece_moves_len;
			break;
		}
	}
}
