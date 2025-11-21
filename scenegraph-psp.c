#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <pspsdk.h>
#include <psploadexec.h>
#include <pspdebugkb.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psputils.h>

#include <pspgu.h>
#include <pspgum.h>

#include "scenegraph.h"
#include "texture.h"

static unsigned int __attribute__((aligned(16))) list[262144];

static int width, height;

void
sg_init(int w, int h)
{
	void *draw_buffer, *disp_buffer, *depth_buffer;

	width = w;
	height = h;
	sceGuInit();
	sceGuStart(GU_DIRECT, list);
	draw_buffer = guGetStaticVramBuffer(512, 272, GU_PSM_8888);
	disp_buffer = guGetStaticVramBuffer(512, 272, GU_PSM_8888);
	depth_buffer = guGetStaticVramBuffer(512, 272, GU_PSM_4444);
	sceGuDrawBuffer(GU_PSM_8888, draw_buffer, 512);
	sceGuDispBuffer(480, 272, disp_buffer, 512);
	sceGuDepthBuffer(depth_buffer, 512);
}

void
sg_init_scenegraph(struct scenegraph *scenegraph)
{
	ScePspFVector3 light0_pos;

	sceGuOffset(2048 - 480/2, 2048 - 272/2);
	sceGuViewport(2048, 2048, 480, 272);
	sceGuScissor(0, 0, 480, 272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuEnable(GU_CULL_FACE);
	sceGuFrontFace(GU_CCW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuDepthRange(65535, 0);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuEnable(GU_BLEND);

	if (scenegraph->light0_enabled) {
		sceGuEnable(GU_LIGHTING);
		sceGuEnable(GU_LIGHT0);
		light0_pos.x = scenegraph->light0_x;
		light0_pos.y = scenegraph->light0_y;
		light0_pos.z = scenegraph->light0_z;
		sceGuLight(0, GU_POINTLIGHT, GU_DIFFUSE_AND_SPECULAR,
			   &light0_pos);
		sceGuLightColor(0, GU_DIFFUSE, scenegraph->light0_color);
		sceGuLightAtt(0, 1, 0, 0);
		sceGuAmbient(GU_COLOR(0.1, 0.1, 0.1, 1));
		sceGuSpecular(1);
	}
	sceGuFinish();
	sceGuSync(0, 0);
	sceGuDisplay(1);
}

void
sg_render(struct scenegraph *scenegraph)
{
	ScePspFVector3 translate = {
		-scenegraph->cam_x, -scenegraph->cam_y, -scenegraph->cam_z
	};
	sceGuStart(GU_DIRECT, list);
	sceGuClearColor(GU_COLOR(0.0, 0.0, 0.0, 1.0));
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	if (scenegraph->cam3d_enabled) {
		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(scenegraph->fov,
				  (float)width/(float)height,
				  scenegraph->near_plane, scenegraph->far_plane);
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();
		sceGumRotateX(-scenegraph->cam_dir_vert);
		sceGumRotateY(-scenegraph->cam_dir_horiz);
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
	}
	sceGumTranslate(&translate);
	for (int i = 0; i < scenegraph->num_render; i++) {
		scenegraph->render[i](scenegraph);
	}
	sceGuFinish();
	sceGuSync(0, 0);
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void
sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj)
{
	uint32_t vert_type;
	ScePspFVector3 translate = {
		obj->x, obj->y, obj->z
	};

	if (obj->flags & SG_OBJ_NOLIGHTDEPTH)
		sceGuDisable(GU_LIGHTING);
	if (obj->flags & SG_OBJ_2D || obj->flags & SG_OBJ_NOLIGHTDEPTH)
		sceGuDisable(GU_DEPTH_TEST);
	vert_type = GU_VERTEX_32BITF;
	if (obj->flags & SG_OBJ_2D) {
		vert_type |= GU_TRANSFORM_2D;
	} else {
		vert_type |= GU_NORMAL_32BITF | GU_TRANSFORM_3D;
	}
	if (obj->flags & SG_OBJ_TEXTURED) {
		struct texture *texture = obj->texture;
		vert_type |= GU_TEXTURE_32BITF;
		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexMode(GU_PSM_8888, 0, 0, 0);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuTexImage(0, texture->width, texture->height,
			      texture->width, texture->buffer);
	}
	sceGuColor(obj->color);
	sceGumPushMatrix();
	sceGumTranslate(&translate);
	sceGumDrawArray(GU_TRIANGLES, vert_type,
			obj->num_vertices, 0, obj->vertices);
	sceGumPopMatrix();
	if (obj->flags & SG_OBJ_TEXTURED)	sceGuDisable(GU_TEXTURE_2D);
	if (obj->flags & SG_OBJ_2D || obj->flags & SG_OBJ_NOLIGHTDEPTH)
		sceGuEnable(GU_DEPTH_TEST);
	if (obj->flags & SG_OBJ_NOLIGHTDEPTH)	sceGuEnable(GU_LIGHTING);
}
