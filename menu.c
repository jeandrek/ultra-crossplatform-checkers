#include <string.h>

#include "checkers.h"
#include "game.h"
#include "menu.h"
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
static char *menu_items[] = {
	"New game",
	/* "Host game", */
	/* "Join game", */
	"Return"
};

static void
menu_render_items(struct scenegraph *scenegraph)
{
	float item_gap = 1.5 / (NUM_MENU_ITEMS + 1);
	float item_y = .75 - item_gap;
	text_scale(1);
	for (int i = 0; i < NUM_MENU_ITEMS; i++) {
		text_color(item_color(i));
		draw_text(scenegraph, menu_items[i], 0, item_y, TEXT_CENTRE);
		item_y -= item_gap;
	}
}

static void (*menu_render_functions[])(struct scenegraph *) = {
	menu_render_items
};

static size_t num_menu_render_functions = 1;

static void
menu_load(void)
{
}

static void
menu_init(void)
{
	memset(&menu.sg, 0, sizeof (menu.sg));
	menu.sg.num_render = num_menu_render_functions;
	menu.sg.render = menu_render_functions;
	sg_init_scenegraph(&menu.sg);
}

static void
menu_update(void)
{
}

static void
menu_input_event(int button)
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

struct state menu = {
	.load = menu_load,
	.init = menu_init,
	.update = menu_update,
	.input_event = menu_input_event
};
