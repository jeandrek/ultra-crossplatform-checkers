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
#include "main_menu.h"

int row_lengths[] = {1, 1};

struct element elems[] = {
	{.x = 0, .y = 0.1, .data = "New game"},
	{.x = 0, .y = -0.1, .data = "Return"}
};

static void
main_menu_render_items(struct scenegraph *scenegraph)
{
	text_scale(1);
	for (int i = 0; i < sizeof (elems)/sizeof (elems[0]); i++) {
		text_color(button_color(i, 0));
		text_draw(scenegraph, elems[i].data, elems[i].x, elems[i].y,
			  TEXT_CENTRE);
	}
}

static void (*main_menu_render_functions[])(struct scenegraph *) = {
	main_menu_render_items
};

static void
click(int row, int col)
{
	switch (row) {
	case 0:
		game.init();
		checkers_switch_state(&game);
		break;
	case 1:
		checkers_switch_state(&game);
		break;
	}
}

static void
main_menu_init(void)
{
	memset(&main_menu.sg, 0, sizeof (main_menu.sg));
	main_menu.sg.num_render = sizeof (main_menu_render_functions)/sizeof (main_menu_render_functions[0]);
	main_menu.sg.render = main_menu_render_functions;
	sg_init_scenegraph(&main_menu.sg);

	for (int i = 0; i < sizeof (elems)/sizeof (elems[0]); i++)
		button_bounds(&main_menu.sg, strlen(elems[i].data),
			      elems[i].x, elems[i].y, &elems[i].bounds);

	gui_set_row_lengths(2, row_lengths);
	gui_set_element(0, 0, &elems[0]);
	gui_set_element(1, 0, &elems[1]);
	gui_set_action_proc(click);
}

struct state main_menu = {
	.init = main_menu_init,
	.update = gui_update,
	.button_event = gui_button_event,
	.mouse_up_event = gui_mouse_up_event
};
