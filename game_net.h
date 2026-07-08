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

#ifndef _GAME_NET_H_
#define _GAME_NET_H_

#include "game.h"

struct disc_ent {
	char		*name;
	/* Private */
	uint32_t	if_idx;
	char		*domain;
	char		*regtype;
};

char	*ip_addr_str(void);

int	game_net_connected(void);

int	game_net_host(int player);
int	game_net_poll_connections(void);
void	game_net_stop_hosting(void);

int	game_net_discover(void);
void	game_net_discovery_update(void);
void	game_net_stop_discovery(void);
void	free_discovered_game(struct disc_ent *disc_ent);
char	*discovered_game_addr(struct disc_ent *disc_ent);
void	game_net_join(const char *nodename);
int	game_net_poll_connected(void);
void	game_net_stop_connecting(void);

int	game_net_poll_move(void);
int	game_net_recv_move(struct move *move);
void	game_net_send_move(struct move move);
void	game_net_disconnect(void);

extern char game_net_player;

#define PROTOCOL_MAJOR	0
#define PROTOCOL_MINOR	3
struct __attribute__ ((packed)) checkers_header {
	uint8_t	magic[2];
	uint8_t	major;
	uint8_t	minor;
};

extern struct other_player other_player_net;

#endif /* !_GAME_NET_H_ */
