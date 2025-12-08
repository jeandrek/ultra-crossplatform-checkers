#include "scenegraph.h"
#include "texture.h"

#ifdef __psp__
#include <pspgu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

static float __attribute__((aligned(16))) sprite_verts[30] = {0};

void
draw_sprite(int left, int top, int width, int height, float centre_x, float centre_y, float scale,
	    struct texture *tex)
{
	float aspect = 800.0 / 450.0;
	float ss_w = 512;
	float ss_h = 256;
#ifdef __psp__
	sprite_verts[0] = left;
	sprite_verts[1] = top;
	sprite_verts[2] = 240 + 240 * centre_x - width / 2.0;
	sprite_verts[3] = 136 - 136 * centre_y - height / 2.0;

	sprite_verts[5] = left + width;
	sprite_verts[6] = top;
	sprite_verts[7] = 240 + 240 * centre_x + width / 2.0;
	sprite_verts[8] = 136 - 136 * centre_y - height / 2.0;

	sprite_verts[10] = left + width;
	sprite_verts[11] = top + height;
	sprite_verts[12] = 240 + 240 * centre_x + width / 2.0;
	sprite_verts[13] = 136 - 136 * centre_y + height / 2.0;

	sprite_verts[15] = left;
	sprite_verts[16] = top;
	sprite_verts[17] = 240 + 240 * centre_x - width / 2.0;
	sprite_verts[18] = 136 - 136 * centre_y - height / 2.0;

	sprite_verts[20] = left + width;
	sprite_verts[21] = top + height;
	sprite_verts[22] = 240 + 240 * centre_x + width / 2.0;
	sprite_verts[23] = 136 - 136 * centre_y + height / 2.0;

	sprite_verts[25] = left;
	sprite_verts[26] = top + height;
	sprite_verts[27] = 240 + 240 * centre_x - width / 2.0;
	sprite_verts[28] = 136 - 136 * centre_y + height / 2.0;
	{
		struct sg_object obj;
		obj.color = 0xffffffff;
		obj.flags = SG_OBJ_2D | SG_OBJ_TEXTURED | SG_OBJ_NOLIGHTDEPTH;
		obj.texture = tex;
		obj.vertices = sprite_verts;
		obj.num_vertices = sizeof (sprite_verts)/(5*sizeof (float));
		sg_render_object(NULL, &obj);
	}
#else
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(-aspect, aspect, -1, 1, -1, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->gl_tex);
	glColor3f(1, 1, 1);

	glBegin(GL_TRIANGLES);
	glTexCoord2f(left/ss_w, (top + height)/ss_h);
	glVertex2f(centre_x - scale*width/450, centre_y - scale*height/450);
	glTexCoord2f((left + width)/ss_w, (top + height)/ss_h);
	glVertex2f(centre_x + scale*width/450, centre_y - scale*height/450);
	glTexCoord2f((left + width)/ss_w, top/ss_h);
	glVertex2f(centre_x + scale*width/450, centre_y + scale*height/450);

	glTexCoord2f(left/ss_w, (top + height)/ss_h);
	glVertex2f(centre_x - scale*width/450, centre_y - scale*height/450);
	glTexCoord2f((left + width)/ss_w, top/ss_h);
	glVertex2f(centre_x + scale*width/450, centre_y + scale*height/450);
	glTexCoord2f(left/ss_w, top/ss_h);
	glVertex2f(centre_x - scale*width/450, centre_y + scale*height/450);
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
#endif
}
