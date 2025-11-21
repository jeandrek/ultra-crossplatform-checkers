#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include <stddef.h>
#include <stdint.h>

struct scenegraph {
	void	(**render)(struct scenegraph *);
	size_t	num_render;
	int	cam3d_enabled;
	float	fov, near_plane, far_plane;
	float	cam_x, cam_y, cam_z;
	float	cam_dir_horiz, cam_dir_vert;
	int	light0_enabled;
	float	light0_x, light0_y, light0_z;
	uint32_t	light0_color;
};

#define SG_OBJ_NOLIGHTING	1<<0
#define SG_OBJ_TEXTURED		1<<1
#define SG_OBJ_2D		1<<2

struct sg_object {
	uint32_t	color;
	uint32_t	flags;
	struct texture	*texture;
	float		*vertices;
	size_t		num_vertices;
	float		x, y, z;
};

void	sg_init(int w, int h);
void	sg_init_scenegraph(struct scenegraph *scenegraph);
void	sg_render(struct scenegraph *scenegraph);
void	sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj);

#ifdef __psp__
#define TEXCOORD(x, l) x
#else
#define TEXCOORD(x, l) x/(float)l
#endif

#endif /* !_SCENEGRAPH_H_ */
