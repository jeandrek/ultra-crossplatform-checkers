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
#if 0
	struct sg_object obj;
	obj.color = (selected_button == 0 ? ~0 : 0xffaaaaaa);
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED | SG_OBJ_NOLIGHTDEPTH;
	obj.texture = &texture_menu_ss;
	obj.vertices = button1_verts;
	obj.num_vertices = sizeof (button1_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0.2;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
	obj.color = (selected_button == 1 ? ~0 : 0xffaaaaaa);
	obj.vertices = button2_verts;
	obj.x = 0;
	obj.y = -0.2;
	sg_render_object(scenegraph, &obj);
#endif
	draw_sprite(0, 0, 72, 24,
		    0, 0.2, 1, &texture_menu_ss);
	draw_sprite(0, 24, 72, 48,
		    0, -0.2, 1, &texture_menu_ss);
}

static void (*menu_render_functions[])(struct scenegraph *) = {
	menu_render_buttons
};

static size_t num_menu_render_functions = 1;

static void
menu_load(void)
{
#ifndef __psp__
	for (int i = 0; i < 30; i += 5) {
		button1_verts[i] /= 512.0;
		button1_verts[i+1] /= 256.0;

		button2_verts[i] /= 512.0;
		button2_verts[i+1] /= 256.0;
	}
#else
	for (int i = 0; i < 30; i += 5) {
		button1_verts[i+2] *= 272;
		button1_verts[i+3] *= 272;

		button2_verts[i+2] *= 272;
		button2_verts[i+3] *= 272;
	}
#endif
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
