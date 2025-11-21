#include "checkers.h"
#include "game.h"
#include "menu.h"

void (*update)(void);

void
checkers_init(void)
{
	game_load();
	menu_load();
	game_init();
}

void
checkers_update(void)
{
	update();
}
