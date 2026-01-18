#ifndef _TEXT_H_
#define _TEXT_H_

enum {
	TEXT_TOPLEFT,
	TEXT_CENTRE
};

void text_init(void);
void text_scale(float new_scale);
void text_color(uint32_t new_color);
void text_draw(struct scenegraph *scenegraph, char *s, float x, float y,
	       int alignment);

#endif /* _TEXT_H_ */
