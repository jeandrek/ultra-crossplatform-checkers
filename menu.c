#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "checkers.h"
#include "menu.h"
#include "scenegraph.h"
#include "texture.h"

struct texture texture_logo;
struct texture texture_button1;
struct texture texture_button2;

/* float __attribute__((aligned(16))) particle_verts[] = { */
/* 	0, 0, 0, */
/* 	0, 3, 0, */
/* 	3, 0, 0, */
/* 	3, 0, 0, */
/* 	0, 3, 0, */
/* 	3, 3, 0 */
/* }; */
float __attribute__((aligned(16))) particle_verts[] = {
	0, 0, 1,
	0, 0, 0,
	0, 0, 1,
	0.05, 0, 0,
	0, 0, 1,
	0, 0.05, 0,
	0, 0, 1,
	0.05, 0, 0,
	0, 0, 1,
	0.05, 0.05, 0,
	0, 0, 1,
	0, 0.05, 0
};
float __attribute__((aligned(16))) particle_verts1[] = {
	0, 0, 0,
	0, 5, 0,
	5, 0, 0,
	5, 0, 0,
	0, 5, 0,
	5, 5, 0
};

static float __attribute__((aligned(16))) logo_verts[] = {
	0, 0,
	0, 0, 0,
	0, 128,
	0, 50, 0,
	256, 0,
	100, 0, 0,
	256, 0,
	100, 0, 0,
	0, 128,
	0, 50, 0,
	256, 128,
	100, 50, 0
};
static float __attribute__((aligned(16))) button1_verts[] = {
	0, 0,
	0, 0, 0,
	0, 64,
	0, 30, 0,
	128, 0,
	60, 0, 0,
	128, 0,
	60, 0, 0,
	0, 64,
	0, 30, 0,
	128, 64,
	60, 30, 0
};
static float __attribute__((aligned(16))) button2_verts[] = {
	0, 0,
	0, 0, 0,
	0, 64,
	0, 30, 0,
	128, 0,
	60, 0, 0,
	128, 0,
	60, 0, 0,
	0, 64,
	0, 30, 0,
	128, 64,
	60, 30, 0
};

int alpha = 0;

void
render_particles(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	static int t = 0;
	static int a = 255;//rand()%127 + 126;
	obj.color=0xffff0000;
	obj.flags = SG_OBJ_NOLIGHTING;
	obj.vertices = particle_verts;
	obj.num_vertices = sizeof (particle_verts)/(6*sizeof (float));
	t++;
	for (int i = 0; i < 300; i++) {
		obj.y = 0;
		obj.z = 3 - i/25.0;
		obj.x = sin(6*obj.z + t/2.0);
		sg_render_object(scenegraph, &obj);
	}
#if 0
	struct sg_object obj;
	static int t = 0;
	static int a = 255;//rand()%127 + 126;
	obj.flags = SG_OBJ_2D;
	obj.vertices = particle_verts1;
	obj.num_vertices = sizeof (particle_verts)/(3*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	t++;
	if (t > 80) {
		if (a > 3)
			a -= 6;
		else a = 0;
	}
	for (int i = 0; i < 100; i++) {
		int p = rand()%10;
		/* if (p == 0) */
		/* 	obj.color = 0xff0000|a<<24; */
		/* else if (p == 1) */
		/* 	obj.color = 0xffffff|a<<24; */
		/* else */ if (p == 2)
			obj.color = 0x0099ff| a << 24;
		else
			obj.color = 0x0000ff|a<<24;
		float r = t<80 ? t/80.0 : 1;
		float b = (rand() % 512) * 2*M_PI/512;
		int x = r*180*cos(b) + 480/2 + rand()%10 - 5;
		int y = r*130*sin(b) + 272/2 + rand()%10 - 5;
		for (int j = 0; j < 18; j+=3) {
			particle_verts1[j] = particle_verts[j] + x;
			particle_verts1[j+1] = particle_verts[j+1] + y;
		}
		sg_render_object(scenegraph, &obj);
	}
#endif
}

void
render_logo(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = 0xffffff | alpha<<24;
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED;
	obj.texture = &texture_logo;
	obj.vertices = logo_verts;
	obj.num_vertices = sizeof (logo_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
}
int selected_button = 0;
void
render_buttons(struct scenegraph *scenegraph)
{
	struct sg_object obj;
	obj.color = (selected_button == 0 ? 0xffffff : 0xaaaaaa) | alpha<<24;
	obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED;
	obj.texture = &texture_button1;
	obj.vertices = button1_verts;
	obj.num_vertices = sizeof (button1_verts)/(5*sizeof (float));
	obj.x = 0;
	obj.y = 0;
	obj.z = 0;
	sg_render_object(scenegraph, &obj);
	obj.color = (selected_button == 1 ? 0xffffff : 0xaaaaaa) | alpha<<24;
	obj.texture = &texture_button2;
	obj.vertices = button2_verts;
	sg_render_object(scenegraph, &obj);
}

void (*menu_render_functions[])(struct scenegraph *) = {
	render_particles,
	render_board,
	render_pieces,
	//render_logo,
	//render_buttons
};

size_t num_menu_render_functions = 3;

void
animate(struct scenegraph *scenegraph)
{
	static int i = 0;
	if (i < 77) {
		scenegraph->cam_z -= 0.25;
		scenegraph->cam_dir_vert -= M_PI/4/80;
		i++;
	} else if (i < 120) i++;
	else if (alpha < 0xff) {
		alpha += 3;
	}
}

void
menu_init(void)
{
	for (int i = 2; i < 30; i += 5) {
		logo_verts[i] += 180;
		logo_verts[i+1] += 40;

		button1_verts[i] += 200;
		button1_verts[i+1] += 100;

		button2_verts[i] += 200;
		button2_verts[i+1] += 140;
	}
	texture_init_from_file(&texture_logo, 256, 128,
			       "assets/textures/logo");
	texture_init_from_file(&texture_button1, 128, 64,
			       "assets/textures/menu1");
	texture_init_from_file(&texture_button2, 128, 64,
			       "assets/textures/menu2");
}
