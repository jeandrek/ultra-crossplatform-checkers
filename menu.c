#include <math.h>
#include <string.h>

#include "checkers.h"
#include "game.h"
#include "menu.h"
#include "sprite.h"
#include "scenegraph.h"
#include "texture.h"
#include "input.h"

static struct texture texture_menu_ss;

int selected_button = 0;

static void
menu_render_buttons(struct scenegraph *scenegraph)
{
	struct sprite button1, button2;

	button1.texture = &texture_menu_ss;
	button1.tex_left = 0;
	button1.tex_top = 0;
	button1.width = 72;
	button1.height = 24;
	button1.x = 0;
	button1.y = 0.2;
	button1.scale = 1;

	button2.texture = &texture_menu_ss;
	button2.tex_left = 0;
	button2.tex_top = 24;
	button2.width = 72;
	button2.height = 24;
	button2.x = 0;
	button2.y = -0.2;
	button2.scale = 1;

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
}

static void
menu_init(void)
{
	bzero(&menu.sg, sizeof (menu.sg));
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
	case INPUT_DOWN:
		selected_button = !selected_button;
		break;
	case INPUT_ACCEPT:
		switch (selected_button) {
		case 0:
			game.init();
		case 1:
			checkers_switch_state(&game);
			break;
		}
		break;
	default:
	}
}

struct state menu = {
	.load = menu_load,
	.init = menu_init,
	.update = menu_update,
	.input_event = menu_input_event
};
