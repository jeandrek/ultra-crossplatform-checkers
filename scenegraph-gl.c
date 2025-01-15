#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "scenegraph.h"
#include "texture.h"

#include <SDL2/SDL.h>

#include <GL/gl.h>
#include <GL/glu.h>

static SDL_Window *window;

void
sg_init(struct scenegraph *scenegraph)
{
	void *draw_buffer, *disp_buffer, *depth_buffer;
	SDL_GLContext context;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == -1) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow("Hello", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  scenegraph->width, scenegraph->height,
				  SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);

	if (!window) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	context = SDL_GL_CreateContext(window);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	if (scenegraph->light0_enabled) {
		GLfloat params[] = {
			scenegraph->light0_x, scenegraph->light0_y,
			scenegraph->light0_z, 1
		};
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glLightfv(GL_LIGHT0, GL_POSITION, params);
	}
}

void
sg_render(struct scenegraph *scenegraph)
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(scenegraph->fov,
		       (float)scenegraph->width/(float)scenegraph->height,
		       scenegraph->near_plane, scenegraph->far_plane);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(-scenegraph->cam_dir_vert * 180/M_PI, 1, 0, 0);
	glRotatef(-scenegraph->cam_dir_horiz * 180/M_PI, 0, 1, 0);
	glTranslatef(-scenegraph->cam_x, -scenegraph->cam_y,
		     -scenegraph->cam_z);
	for (int i = 0; i < scenegraph->num_render; i++) {
		scenegraph->render[i](scenegraph);
	}
	SDL_GL_SwapWindow(window);
}

void
sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj)
{
	if (obj->flags & SG_OBJ_NOLIGHTING)	glDisable(GL_LIGHTING);
	if (obj->flags & SG_OBJ_TEXTURED) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, obj->texture->gl_tex);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	glColor4f((obj->color & 0xff)/255.0,
		  ((obj->color >> 8) & 0xff)/255.0,
		  ((obj->color >> 16) & 0xff)/255.0,
		  (obj->color >> 24)/255.0);
	glPushMatrix();
	glTranslatef(obj->x, obj->y, obj->z);
	if (obj->flags & SG_OBJ_TEXTURED) {
		int stride = 8*sizeof (float);
		glTexCoordPointer(2, GL_FLOAT, stride, obj->vertices);
		glNormalPointer(GL_FLOAT, stride, &obj->vertices[2]);
		glVertexPointer(3, GL_FLOAT, stride, &obj->vertices[5]);
	} else {
		int stride = 6*sizeof (float);
		glNormalPointer(GL_FLOAT, stride, obj->vertices);
		glVertexPointer(3, GL_FLOAT, stride, &obj->vertices[3]);
	}
	glDrawArrays(GL_TRIANGLES, 0, obj->num_vertices);
	glPopMatrix();
	if (obj->flags & SG_OBJ_TEXTURED) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
	if (obj->flags & SG_OBJ_NOLIGHTING)	glEnable(GL_LIGHTING);
}
