#include <string.h>

#include "checkers.h"
#include "game.h"
#include "menu.h"
#include "sprite.h"
#include "scenegraph.h"
#include "texture.h"
#include "input.h"

static struct texture texture_menu_ss;

static struct sprite button1;
static struct sprite button2;

int selected_button = 0;

static void
menu_render_buttons(struct scenegraph *scenegraph)
{
	if (selected_button == 0) {
		button1.base_color = 0xffffffff;
		button2.base_color = 0xffaaaaaa;
	} else {
		button1.base_color = 0xffaaaaaa;
		button2.base_color = 0xffffffff;
	}
	sprite_draw(scenegraph, &button1);
	sprite_draw(scenegraph, &button2);
}

static void (*menu_render_functions[])(struct scenegraph *) = {
	menu_render_buttons
};

static size_t num_menu_render_functions = 1;

static void
menu_load(void)
{
	texture_init_from_file(&texture_menu_ss, 512, 256,
			       "assets/textures/menu-ss");

	sprite_init(&button1, &texture_menu_ss, 0, 0, 72, 24);
	button1.x = 0;
	button1.y = 0.2;
	button1.scale = 1;

	sprite_init(&button2, &texture_menu_ss, 0, 24, 72, 24);
	button2.x = 0;
	button2.y = -0.2;
	button2.scale = 1;
}

static void
menu_init(void)
{
	memset(&menu.sg, 0, sizeof (menu.sg));
	menu.sg.num_render = num_menu_render_functions;
	menu.sg.render = menu_render_functions;
	sg_init_scenegraph(&menu.sg);

	if (menu.sg.height > 300) {
		button1.scale = 2;
		button2.scale = 2;
	}
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
	case INPUT_DOWN:
		selected_button = !selected_button;
		break;
	case INPUT_ACCEPT:
		switch (selected_button) {
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
