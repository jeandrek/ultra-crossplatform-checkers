#include "checkers.h"
#include "game.h"
#include "menu.h"
#include "scenegraph.h"

void (*update)(struct scenegraph *);

void
checkers_init(struct scenegraph *scenegraph)
{
	menu_load(scenegraph);
	menu_init(scenegraph);
}

void
checkers_update(struct scenegraph *scenegraph)
{
	update(scenegraph);
}
