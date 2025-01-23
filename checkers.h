#ifndef _CHECKERS_H_
#define _CHECKERS_H_

struct scenegraph;

void checkers_init(struct scenegraph *scenegraph, int width, int height);
void render_board(struct scenegraph *scenegraph);
void render_pieces(struct scenegraph *scenegraph);
void render_piece(struct scenegraph *scenegraph, float x, float y, float z,
		  uint32_t color);

#endif /* !_CHECKERS_H_ */
