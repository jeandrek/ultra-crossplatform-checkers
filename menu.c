/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include "checkers.h"
#include "game.h"
#include "game_checkers.h"
#include "game_computer.h"
#include "scenegraph.h"
#include "text.h"
#include "gui.h"
#include "net_menu.h"
#include "menu.h"

struct element *elems;
static int num_elems;
static char *message = NULL;

static void confirm_dlg(void (*yes_action)(void));

static void
new_2player_game(void)
{
	game_type = LOCAL_2PLAYER;
	game.destroy();
	game.init();
	checkers_switch_state(&game);
}

static void
new_computer_game(int player)
{
	game_type = COMPUTER;
	game_computer_player = !player;
	game.destroy();
	game.init();
	checkers_switch_state(&game);
}

static void
new_game_menu_action(int row, int col)
{
	switch (row) {
	case 0: new_2player_game(); break;
	case 1: new_computer_game(0); break;
	case 2: new_computer_game(1); break;
	case 3: main_menu(); break;
	}
}

static void
new_game_menu(void)
{
	static struct element new_game_menu_elems[] = {
		{.x = 0, .y = 0.3, .data = "Player vs. player"},
		{.x = 0, .y = 0.1, .data = "Play as red vs. computer"},
		{.x = 0, .y = -0.1, .data = "Play as black vs. computer"},
		{.x = 0, .y = -0.3, .data = "Back"}
	};
	menu_set_elements(4, new_game_menu_elems);
	gui_set_rows(4, 1, &elems[0], 1, &elems[1], 1, &elems[2], 1, &elems[3]);
	gui_set_action_proc(new_game_menu_action);
}

static void
main_menu_action(int row, int col)
{
	switch (row) {
	case 0:
		if (game_dirty) confirm_dlg(new_game_menu);
		else new_game_menu();
		break;

	case 1:
		if (game_dirty) confirm_dlg(net_menu);
		else net_menu();
		break;

	case 2:
		checkers_switch_state(&game);
		break;
	}
}

void
main_menu(void)
{
	static struct element main_menu_elems[] = {
		{.x = 0, .y = 0.2, .data = "New game~"},
		{.x = 0, .y = 0, .data = "Network game~"},
		{.x = 0, .y = -0.2, .data = "Return"}
	};
	menu_set_elements(3, main_menu_elems);
	gui_set_rows(3, 1, &elems[0], 1, &elems[1], 1, &elems[2]);
	gui_set_action_proc(main_menu_action);
}

static void (*message_dlg_back_action)(void);

static void message_dlg_action(int row, int col) {message_dlg_back_action();}

void
message_dlg(char *text, void (*back_action)(void))
{
	static struct element message_dlg_elems[] = {
		{.x = 0, .y = -0.1, .data = "Back"}
	};
	message_dlg_back_action = back_action;
	menu_set_elements(1, message_dlg_elems);
	message = text;
	gui_set_rows(1, 1, &elems[0]);
	gui_set_action_proc(message_dlg_action);
}

static void (*confirm_dlg_yes_action)(void);

static void
confirm_dlg_action(int row, int col)
{
	switch (col) {
	case 0: confirm_dlg_yes_action(); break;
	case 1: main_menu(); break;
	}
}

static void
confirm_dlg(void (*yes_action)(void))
{
	static struct element confirm_dlg_elems[] = {
		{.x = -0.2, .y = -0.1, .data = "Yes"},
		{.x = 0.2, .y = -0.1, .data = "No"}
	};
	confirm_dlg_yes_action = yes_action;
	menu_set_elements(2, confirm_dlg_elems);
	message = "Starting a game will quit current game.  Continue?";
	gui_set_rows(1, 2, &elems[0], &elems[1]);
	gui_set_action_proc(confirm_dlg_action);
}

void
menu_set_elements(int num, struct element *new_elems)
{
	num_elems = num;
	elems = new_elems;
	message = NULL;
	for (int i = 0; i < num_elems; i++)
		button_bounds(&menu.sg, strlen(elems[i].data),
			      elems[i].x, elems[i].y, &elems[i].bounds);
}

void
menu_render_items(struct scenegraph *scenegraph)
{
	text_size(1);
	for (int i = 0; i < num_elems; i++) {
		text_color(button_color(elems[i].row, elems[i].col));
		text_draw(scenegraph, elems[i].data, elems[i].x, elems[i].y,
			  TEXT_CENTRE);
	}
}

static void
menu_render_message(struct scenegraph *scenegraph)
{
	if (message == NULL) return;
	text_size(1);
	text_color(0xffaaaaaa);
	text_draw(scenegraph, message, 0, 0.1, TEXT_CENTRE);
}

static void (*menu_render_functions[])(struct scenegraph *) = {
	menu_render_items,
	menu_render_message
};

static void
menu_init(void)
{
	memset(&menu.sg, 0, sizeof (menu.sg));
	menu.sg.num_render = sizeof (menu_render_functions)/sizeof (menu_render_functions[0]);
	menu.sg.render = menu_render_functions;
	sg_init_scenegraph(&menu.sg);
	main_menu();
}

struct state menu = {
	.init = menu_init,
	.update = gui_update,
	.button_event = gui_button_event,
	.mouse_up_event = gui_mouse_up_event,
	.mouse_move_event = gui_mouse_move_event
};
