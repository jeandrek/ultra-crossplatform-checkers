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
#include "main_menu.h"
#include "scenegraph.h"
#include "texture.h"
#include "input.h"
#include "text.h"

static int selected_item = 0;

static uint32_t
item_color(int n)
{
	return n == selected_item ? 0xffffffff : 0xffaaaaaa;
}

#define NUM_MENU_ITEMS 2
static char *main_menu_items[] = {
	"New game",
	/* "Host game", */
	/* "Join game", */
	"Return"
};

static void
main_menu_render_items(struct scenegraph *scenegraph)
{
	float item_gap = 1.5 / (NUM_MENU_ITEMS + 1);
	float item_y = .75 - item_gap;
	text_scale(1);
	for (int i = 0; i < NUM_MENU_ITEMS; i++) {
		text_color(item_color(i));
		text_draw(scenegraph, main_menu_items[i], 0, item_y, TEXT_CENTRE);
		item_y -= item_gap;
	}
}

static void (*main_menu_render_functions[])(struct scenegraph *) = {
	main_menu_render_items
};

static size_t num_main_menu_render_functions = 1;

static void
main_menu_load(void)
{
}

static void
main_menu_init(void)
{
	memset(&main_menu.sg, 0, sizeof (main_menu.sg));
	main_menu.sg.num_render = num_main_menu_render_functions;
	main_menu.sg.render = main_menu_render_functions;
	sg_init_scenegraph(&main_menu.sg);
}

static void
main_menu_update(void)
{
}

static void
main_menu_input_event(int button)
{
	switch (button) {
	case INPUT_UP:
		selected_item = (selected_item == 0 ?
				 NUM_MENU_ITEMS - 1 : selected_item - 1);
		break;
	case INPUT_DOWN:
		selected_item = (selected_item + 1) % NUM_MENU_ITEMS;
		break;
	case INPUT_ACCEPT:
		switch (selected_item) {
		case 0:
			game.init();
			checkers_switch_state(&game);
			break;
		case 1:
			checkers_switch_state(&game);
			break;
		}
		break;
	}
}

struct state main_menu = {
	.load = main_menu_load,
	.init = main_menu_init,
	.update = main_menu_update,
	.input_event = main_menu_input_event
};
