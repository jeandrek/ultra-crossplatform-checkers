#ifndef _MENU_H_
#define _MENU_H_

void menu_init(void);
extern void (*menu_render_functions[])(struct scenegraph *);
extern size_t num_menu_render_functions;

#endif /* !_MENU_H_ */
