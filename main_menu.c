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
