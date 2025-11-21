#include <math.h>
#include <string.h>

#include "checkers.h"
#include "game.h"
#include "menu.h"
#include "scenegraph.h"
#include "texture.h"
#include "input.h"

static struct texture texture_menu_ss;

static float __attribute__((aligned(16))) button1_verts[] = {
	0, 0,
	0, 0, 0,
	0, 25,
	0, 25, 0,
	75, 0,
	75, 0, 0,
	75, 0,
	75, 0, 0,
	0, 25,
	0, 25, 0,
	75, 25,
	75, 25, 0
};
static float __attribute__((aligned(16))) button2_verts[] = {
	0, 25,
	0, 0, 0,
	0, 50,
	0, 25, 0,
	75, 25,
	75, 0, 0,
	75, 25,
	75, 0, 0,
	0, 50,
	0, 25, 0,
	75, 50,
	75, 25, 0
};

int selected_button = 0;

static void
menu_render_buttons(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = (selected_button == 0 ? ~0 : 0xffaaaaaa);
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED | SG_OBJ_NOLIGHTING;
	obj.texture = &texture_menu_ss;
	obj.vertices = button1_verts;
	obj.num_vertices = sizeof (button1_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
	obj.color = (selected_button == 1 ? ~0 : 0xffaaaaaa);
	obj.vertices = button2_verts;
	sg_render_object(scenegraph, &obj);
}

static void (*menu_render_functions[])(struct scenegraph *) = {
	menu_render_buttons
};

static size_t num_menu_render_functions = 1;

static void
menu_load(void)
{
	for (int i = 2; i < 30; i += 5) {
		button1_verts[i] += 202;
		button1_verts[i+1] += 100;

		button2_verts[i] += 202;
		button2_verts[i+1] += 150;
	}
#ifndef __psp__
	for (int i = 0; i < 30; i += 5) {
		button1_verts[i] /= 512.0;
		button1_verts[i+1] /= 256.0;
		button1_verts[i+2] /= 240.0;
		button1_verts[i+2] -= 1;
		button1_verts[i+3] /= -136.0;
		button1_verts[i+3] += 1;
		button2_verts[i] /= 512.0;
		button2_verts[i+1] /= 256.0;
		button2_verts[i+2] /= 240.0;
		button2_verts[i+2] -= 1;
		button2_verts[i+3] /= -136.0;
		button2_verts[i+3] += 1;
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
	switch (input_read()) {
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
	.update = menu_update
};
