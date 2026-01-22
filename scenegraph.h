/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
	int	width, height;	/* XXX */
};

#define SG_OBJ_NOLIGHTDEPTH	(1<<0)
#define SG_OBJ_TEXTURED		(1<<1)

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

#endif /* !_SCENEGRAPH_H_ */
