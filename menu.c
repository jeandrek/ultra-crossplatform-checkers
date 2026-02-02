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
#include "scenegraph.h"
#include "texture.h"
#include "input.h"
#include "text.h"
#include "gui.h"
#include "menu.h"

int game_dirty = 1;

static int num_elems;
static struct element *elems;
static char *message = NULL;

static void menu_set_bounds(void);

static void main_menu(void);
static void confirm_dlg(void);

static void
main_menu_action(int row, int col)
{
	switch (row) {
	case 0:
		if (game_dirty) {
			confirm_dlg();
		} else {
			game.init();
			checkers_switch_state(&game);
		}
		break;

	case 1:
		checkers_switch_state(&game);
		break;
	}
}

static void
main_menu(void)
{
	static struct element main_menu_elems[] = {
		{.x = 0, .y = 0.1, .data = "New game"},
		{.x = 0, .y = -0.1, .data = "Return"}
	};
	num_elems = 2;
	elems = main_menu_elems;
	message = NULL;
	menu_set_bounds();
	gui_set_rows(2, 1, &elems[0], 1, &elems[1]);
	gui_set_action_proc(main_menu_action);
}

static void
confirm_dlg_action(int row, int col)
{
	switch (col) {
	case 0:
		game.init();
		checkers_switch_state(&game);
		break;
	case 1:
		main_menu();
		break;
	}
}

static void
confirm_dlg(void)
{
	static struct element table_elems[] = {
		{.x = -0.2, .y = -0.1, .data = "OK"},
		{.x = 0.2, .y = -0.1, .data = "Cancel"}
	};
	num_elems = 2;
	elems = table_elems;
	message = "Starting a game will quit current game.  Continue?";
	menu_set_bounds();
	gui_set_rows(1, 2, &elems[0], &elems[1]);
	gui_set_action_proc(confirm_dlg_action);
}

static void
menu_set_bounds(void)
{
	for (int i = 0; i < num_elems; i++)
		button_bounds(&menu.sg, strlen(elems[i].data),
			      elems[i].x, elems[i].y, &elems[i].bounds);
}

static void
menu_render_items(struct scenegraph *scenegraph)
{
	text_scale(1);
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
	text_scale(1);
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
	.mouse_up_event = gui_mouse_up_event
};
