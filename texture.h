#ifndef _TEXTURE_H_
#define _TEXTURE_H_

struct texture {
	int	width;
	int	height;
#ifdef __psp__
	void	*buffer;
#else
	unsigned	gl_tex;
#endif
};

void	texture_init(struct texture *texture, int width, int height,
		     void *pixels);
void	texture_init_from_file(struct texture *texture, int width, int height,
			       char *path);

#endif /* !_TEXTURE_H_ */
