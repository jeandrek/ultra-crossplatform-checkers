#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

struct scenegraph {
	void	(**render)(struct scenegraph *);
	size_t	num_render;
	int	width;
	int	height;
	float	fov, near_plane, far_plane;
	float	cam_x, cam_y, cam_z;
	float	cam_dir_horiz, cam_dir_vert;
	int	light0_enabled;
	float	light0_x, light0_y, light0_z;
	uint32_t	light0_color;
};

#define SG_OBJ_NOLIGHTING	1<<0
#define SG_OBJ_TEXTURED		1<<1

struct sg_object {
	uint32_t	color;
	uint32_t	flags;
	struct texture	*texture;
	float		*vertices;
	size_t		num_vertices;
	float		x, y, z;
};

void	sg_init(struct scenegraph *scenegraph);
void	sg_render(struct scenegraph *scenegraph);
void	sg_render_object(struct scenegraph *scenegraph, struct sg_object *obj);

#endif /* !_SCENEGRAPH_H_ */
