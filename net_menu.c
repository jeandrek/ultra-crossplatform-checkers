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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_net.h"
#include "scenegraph.h"
#include "text.h"
#include "gui.h"
#include "text_input.h"
#include "sprite.h"
#include "menu.h"
#include "net_menu.h"

static void host_menu(void);
static void join_menu(void);

static void
net_menu_action(int row, int col)
{
	switch (row) {
	case 0: host_menu(); break;
	case 1: join_menu(); break;
	case 2: main_menu(); break;
	}
}

void
net_menu(void)
{
	static struct element net_menu_elems[] = {
		{.x = 0, .y = 0.2, .data = "Host game~"},
		{.x = 0, .y = 0, .data = "Join game~"},
		{.x = 0, .y = -0.2, .data = "Back"}
	};
	menu_set_elements(3, net_menu_elems);
	gui_set_rows(3, 1, &elems[0], 1, &elems[1], 1, &elems[2]);
	gui_set_action_proc(net_menu_action);
}

static void
quit_hosting(void)
{
	menu.update = gui_update;
	game_net_stop_hosting();
	host_menu();
}

static void
update_hosting_wait_screen(void)
{
	gui_update();
	if (game_net_poll_connections()) {
		menu.update = gui_update;
		game.init();
		checkers_switch_state(&game);
	}
}

static void
host_game(int player)
{
	static char wait_screen_msg[128];

	game.destroy();
	if (game_net_host(player)) {
		snprintf(wait_screen_msg, 128,
			 "Waiting for connection~  IP address: %s",
			 ip_addr_str());
		menu.update = update_hosting_wait_screen;
		message_dlg(wait_screen_msg, quit_hosting);
	} else {
		message_dlg("Error hosting", host_menu);
	}
}

static void
cancel_connecting(void)
{
	menu.update = gui_update;
	game_net_stop_connecting();
	join_menu();
}

static char *join_game_addr;

static void
update_connecting_wait_screen(void)
{
	static char error_msg[128];
	int result;

	gui_update();
	if ((result = game_net_poll_connected()) > 0) {
		free(join_game_addr);
		menu.update = gui_update;
		game.init();
		checkers_switch_state(&game);
	} else if (result < 0) {
		snprintf(error_msg, 128, "Error connecting to %s",
			 join_game_addr);
		free(join_game_addr);
		menu.update = gui_update;
		message_dlg(error_msg, join_menu);
	}
}

static void
join_game(const char *addr)
{
	static char wait_screen_msg[128];

	game.destroy();
	game_net_join(addr);
	menu.update = update_connecting_wait_screen;
	snprintf(wait_screen_msg, 128, "Connecting to %s~", addr);
	join_game_addr = strdup(addr);
	message_dlg(wait_screen_msg, cancel_connecting);
}

static void
join_address_entry(void)
{
	text_input("IP address/Host", join_game, join_menu);
}

static void
host_menu_action(int row, int col)
{
	switch (row) {
	case 0: host_game(0); break;
	case 1: host_game(1); break;
	case 2: net_menu(); break;
	}
}

static void
host_menu(void)
{
	static struct element host_menu_elems[] = {
		{.x = 0, .y = 0.2, .data = "Play as red~"},
		{.x = 0, .y = 0, .data = "Play as black~"},
		{.x = 0, .y = -0.2, .data = "Back"}
	};
	menu_set_elements(3, host_menu_elems);
	gui_set_rows(3, 1, &elems[0], 1, &elems[1], 1, &elems[2]);
	gui_set_action_proc(host_menu_action);
}

static struct join_item {
	struct disc_ent *disc_ent;
	struct element elem;
	struct join_item *next;
} *join_items;
static int num_join_items;
static int discovery_available;

static void
quit_browsing(void)
{
	struct join_item *next;
	for (struct join_item *item = join_items; item; item = next) {
		next = item->next;
		free_discovered_game(item->disc_ent);
		free(item);
	}
	game_net_stop_discovery();
	menu.update = gui_update;
	menu.sg.render[0] = menu_render_items;
}

static void
join_menu_action(int row, int col)
{
	if (row == 0) {
		quit_browsing();
		join_address_entry();
	} else if (row == num_join_items + 1) {
		quit_browsing();
		net_menu();
	} else {
		struct join_item *item = join_items;
		char *addr;
		for (int idx = row - 1; idx > 0; --idx)
			item = item->next;
		addr = discovered_game_addr(item->disc_ent);
		quit_browsing();
		join_game(addr);
		free(addr);
	}
}

void
show_discovered_games(void)
{
	float y = 0.5;
	gui_set_rows(0);
	gui_add_row(1, &elems[0]);
	for (struct join_item *item = join_items; item; item = item->next) {
		item->elem.x = 0;
		item->elem.y = y;
		button_bounds(&menu.sg, strlen(item->disc_ent->name),
			      item->elem.x, item->elem.y, &item->elem.bounds);
		gui_add_row(1, &item->elem);
		y -= 0.2;
	}
	gui_add_row(1, &elems[1]);
}

void
add_discovered_game(struct disc_ent *ent)
{
	struct join_item *item;

	for (item = join_items; item; item = item->next) {
		if (!strcmp(item->disc_ent->name, ent->name)) {
			free_discovered_game(ent);
			return;
		}
	}
	item = malloc(sizeof (*item));
	item->disc_ent = ent;
	item->next = join_items;
	join_items = item;
	num_join_items++;
}

void
remove_discovered_game(const char *name)
{
	struct join_item *prev = NULL;
	for (struct join_item *item = join_items; item; item = item->next) {
		if (!strcmp(item->disc_ent->name, name)) {
			if (item == join_items)
				join_items = item->next;
			else
				prev->next = item->next;
			free_discovered_game(item->disc_ent);
			free(item);
			num_join_items--;
			return;
		}
		prev = item;
	}
}

static void
join_menu_update(void)
{
	gui_update();
	game_net_discovery_update();
}

static struct sprite discovered_games_box;

static void
join_menu_render_items(struct scenegraph *scenegraph)
{
	menu_render_items(scenegraph);
	if (!discovery_available)
		return;
	sprite_draw(scenegraph, &discovered_games_box);
	text_color(0xffaaaaaa);
	text_draw(scenegraph, "Discovered games:", 0, 0.7, TEXT_CENTRE);
	for (struct join_item *item = join_items; item; item = item->next) {
		text_color(button_color(item->elem.row, item->elem.col, 0));
		text_draw(scenegraph, item->disc_ent->name,
			  item->elem.x, item->elem.y,
			  TEXT_CENTRE);
	}
}

static void
join_menu(void)
{
	static struct element join_menu_elems[] = {
		{.x = 0, .y = 0.9, .data = "Enter address~"},
		{.x = 0, .y = -0.9, .data = "Back"}
	};
	discovery_available = game_net_discover();
	if (!discovery_available) {
		join_menu_elems[0].y = 0.1;
		join_menu_elems[1].y = -0.1;
	}
	num_join_items = 0;
	join_items = NULL;
	menu.update = join_menu_update;
	menu.sg.render[0] = join_menu_render_items;
	menu_set_elements(2, join_menu_elems);
	gui_set_rows(2, 1, &elems[0], 1, &elems[1]);
	gui_set_action_proc(join_menu_action);
	sprite_init(&discovered_games_box, NULL, 0, 0,
		    sg_width/2, 8*sg_height/10);
	discovered_games_box.base_color = 0xff101010;
}
