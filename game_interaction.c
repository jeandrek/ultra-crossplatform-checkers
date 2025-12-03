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
int player_turn = 0;
static enum {
	SELECT_PIECE,
	SELECT_MOVE
} cur_mode;

static void
set_sel_square(int i)
{
	sel_square = i;
	if ((board[player_turn] >> i) & 1)
		sel_piece_moves_len = piece_moves(sel_piece_moves, i);
	else
		sel_piece_moves_len = 0;
}

static void
move_sel_square(int num)
{
	if (sel_square + num > 63)	set_sel_square(63);
	else if (sel_square + num < 0)	set_sel_square(0);
	else				set_sel_square(sel_square + num);
}

void
game_interaction_init(void)
{
	cur_mode = SELECT_PIECE;
	set_sel_square(0);
}

static void
move_piece(void)
{
	board[player_turn] ^= (uint64_t)1<<(uint64_t)sel_square;
	sel_square = sel_piece_moves[sel_move_idx];
	board[player_turn] |= (uint64_t)1<<(uint64_t)sel_square;
	sel_move_idx = 0;
	cur_mode = SELECT_PIECE;
	player_turn = !player_turn;
	set_sel_square(player_turn == 0 ? 0 : 63);
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
			if ((board[player_turn] >> sel_square) & 1) {
				cur_mode = SELECT_MOVE;
				sel_move_idx = (player_turn == 0 ?
						0 : sel_piece_moves_len - 1);
			}
			break;
		case INPUT_UP:
			move_sel_square(player_turn == 0 ? 8 : -8);
			break;
		case INPUT_DOWN:
			move_sel_square(player_turn == 0 ? -8 : 8);
			break;
		case INPUT_LEFT:
			move_sel_square(player_turn == 0 ? -1 : 1);
			break;
		case INPUT_RIGHT:
			move_sel_square(player_turn == 0 ? 1 : -1);
			break;
		}
	} else if (cur_mode == SELECT_MOVE) {
		switch (button) {
		case INPUT_BACK:
			cur_mode = SELECT_PIECE;
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
