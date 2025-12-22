#include "checkers.h"
#include "game.h"
#include "game_interaction.h"
#include "game_checkers.h"
#include "menu.h"
#include "input.h"

struct move board_moves[64][MAX_MOVES];
int board_num_moves[64];

int sel_square;
int sel_piece_type;
int sel_piece_moves_len;
struct move *sel_piece_moves;
int sel_move_idx;
int player_turn = 0;

static void
set_sel_square(int i)
{
	sel_square = i;
	sel_piece_type =
		piece_occupying_square_belonging_to_player(i, player_turn);
	sel_piece_moves = board_moves[i];
	sel_piece_moves_len = board_num_moves[i];
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
	player_turn = 0;
	cur_mode = SELECT_PIECE;
	board_available_moves(board_moves, board_num_moves, player_turn);
	set_sel_square(0);
}

static void
move_piece(void)
{
	perform_move(&sel_piece_moves[sel_move_idx]);
	sel_move_idx = 0;
	player_turn = !player_turn;
	board_available_moves(board_moves, board_num_moves, player_turn);
	game_start_anim_rotate();
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
			if ((board[player_turn][sel_piece_type] >> sel_square) & 1) {
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
			if (sel_piece_moves_len > 0)
				move_piece();
			break;
		case INPUT_UP:
		case INPUT_DOWN:
		case INPUT_LEFT:
		case INPUT_RIGHT:
			if (sel_piece_moves_len > 0)
				sel_move_idx = (sel_move_idx + 1) % sel_piece_moves_len;
			break;
		}
	}
}
