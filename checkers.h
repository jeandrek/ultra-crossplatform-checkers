#ifndef _CHECKERS_H_
#define _CHECKERS_H_

struct scenegraph;

extern void (*update)(struct scenegraph *);

void checkers_init(struct scenegraph *scenegraph);
void checkers_update(struct scenegraph *scenegraph);

#endif /* !_CHECKERS_H_ */
