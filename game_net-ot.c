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

#include <string.h>
#include <stdlib.h>
#include <Carbon.h>
#include "checkers.h"
#include "extra_carbon_defs.h"
#include "net_menu.h"
#include "game_net.h"

#define ntohl(x) x
#define htonl(x) x
#define ntohll(x) x
#define htonll(x) x

static int	game_net_connect_to_client(void);
static void	game_net_send_header(void);
static int	game_net_check_header(void);

EndpointRef	conn_endpoint = NULL;
static int	conn_sock_state;
enum {CANT_CONNECT, CONNECTING, WAITING_FOR_HEADER, WAITING_FOR_OPPONENT};

#ifdef USE_BONJOUR
static DNSServiceRef sd_register = NULL;
static DNSServiceRef sd_browse = NULL;
static DNSServiceRef sd_resolve = NULL;
#endif
char		game_net_player = -1;

char *
ip_addr_str(void)
{
	return "";
}

int
game_net_connected(void)
{
	return conn_endpoint != NULL;
}

int
game_net_host(int player)
{
	return 0;
}

/*
 * Alternatively could use another thread and blocking operations.
 */

int
game_net_poll_connections(void)
{
	return 0;
}

static int
game_net_connect_to_client(void)
{
	return 0;
}

void
game_net_stop_hosting(void)
{
}

#ifdef USE_BONJOUR
static void
discovery_callback(DNSServiceRef sd_ref, DNSServiceFlags flags,
		   uint32_t if_idx, DNSServiceErrorType error,
		   const char *name, const char *regtype,
		   const char *domain, void *context)
{
	if (!error) {
		if (flags & kDNSServiceFlagsAdd) {
			struct disc_ent *ent;
			ent = malloc(sizeof (*ent));
			ent->name = strdup(name);
			ent->if_idx = if_idx;
			ent->domain = strdup(domain);
			ent->regtype = strdup(regtype);
			add_discovered_game(ent);
			if (!(flags & kDNSServiceFlagsMoreComing))
				show_discovered_games();
		} else {
			remove_discovered_game(name);
			show_discovered_games();
		}
	}
}
#endif

int
game_net_discover(void)
{
#ifdef USE_BONJOUR
	DNSServiceErrorType err;
	err = DNSServiceBrowse(&sd_browse, 0, 0, "_checkers._tcp", NULL,
			       discovery_callback, NULL);
	return !err;
#else
	return 0;
#endif
}

void
game_net_discovery_update(void)
{
#ifdef USE_BONJOUR
	struct timeval timeout = {0, 0};
	fd_set fds;
	int sock;

	if (sd_browse == NULL)
		return;
	sock = DNSServiceRefSockFD(sd_browse);
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	if (select(sock + 1, &fds, NULL, NULL, &timeout))
		DNSServiceProcessResult(sd_browse);
#endif
}

void
game_net_stop_discovery(void)
{
#ifdef USE_BONJOUR
	if (sd_browse != NULL)
		DNSServiceRefDeallocate(sd_browse);
	sd_browse = NULL;
#endif
}

void
free_discovered_game(struct disc_ent *disc_ent)
{
	free(disc_ent->name);
	free(disc_ent->domain);
	free(disc_ent->regtype);
	free(disc_ent);
}

#ifdef USE_BONJOUR
static char *resolved_host;

static void
resolution_callback(DNSServiceRef sd_ref, DNSServiceFlags flags,
		    uint32_t if_idx, DNSServiceErrorType error,
		    const char *fullname, const char *hosttarget,
		    uint16_t port, uint16_t txt_len,
		    const unsigned char *txt_record,
		    void *context)
{
	resolved_host = strdup(hosttarget);
}
#endif

char *
discovered_game_addr(struct disc_ent *disc_ent)
{
#ifdef USE_BONJOUR
	DNSServiceErrorType err;

	err = DNSServiceResolve(&sd_resolve, 0, disc_ent->if_idx,
				disc_ent->name, disc_ent->regtype,
				disc_ent->domain,
				(DNSServiceResolveReply)resolution_callback,
				NULL);
	if (err)
		return NULL;
	DNSServiceProcessResult(sd_resolve);
	DNSServiceRefDeallocate(sd_resolve);
	return resolved_host;
#else
	return NULL;
#endif
}

typedef struct {
	uint32_t maxlen, len;
	uint8_t *buf;
} TNetbuf;

typedef struct {
	TNetbuf addr, opt, udata;
	uint32_t sequence;
} TCall;

typedef struct {
	uint16_t type;
	char name[256];
} DNSAddress;

OSStatus join_error = 0;

void
game_net_join(const char *nodename)
{
	TCall conn;
	size_t len;
	DNSAddress addr;
	OSStatus err;

	conn_endpoint = OTOpenEndpointInContext(OTCreateConfiguration("tcp"),
						0, NULL, &err, ot);
	if (err != 0) {
		join_error = err;
		conn_sock_state = CANT_CONNECT;
		return;
	}

	OTSetBlocking(conn_endpoint);
	OTSetSynchronous(conn_endpoint);
	OTBind(conn_endpoint, NULL, NULL);

	OTMemzero(&conn, sizeof (conn));
	len = OTInitDNSAddress(&addr, nodename);
	conn.addr.len = len;
	conn.addr.buf = &addr;

	if ((err = OTConnect(conn_endpoint, &conn, NULL)) != 0) {
		join_error = err;
		conn_sock_state = CANT_CONNECT;
		return;
	}

	conn_sock_state = CONNECTING;
	join_error = 0;
	game_net_send_header();

	if (!game_net_check_header()) {
		OTCloseProvider(conn_endpoint);
		conn_sock_state = CANT_CONNECT;
		return;
	}

	char opponent;
	if (OTRcv(conn_endpoint, &opponent, 1, 0) != 1) {
		OTCloseProvider(conn_endpoint);
		conn_sock_state = CANT_CONNECT;
		return;
	}

	game_net_player = !opponent;
}

int
game_net_poll_connected(void)
{
	if (conn_sock_state == CANT_CONNECT) {
		return join_error;
	}
	return 0;
}

void
game_net_stop_connecting(void)
{
}

static void
game_net_send_header(void)
{
	struct checkers_header header;

	header.magic[0] = 0xff;
	header.magic[1] = 'C';
	header.major = PROTOCOL_MAJOR;
	header.minor = PROTOCOL_MINOR;
	OTSnd(conn_endpoint, (char *)&header, sizeof (header), 0);
}

static int
game_net_check_header(void)
{
	struct checkers_header header;

	if (OTRcv(conn_endpoint, (char *)&header, sizeof (header), 0)
	    < sizeof (header))
		return 0;
	if (header.magic[0] != 0xff || header.magic[1] != 'C'
	    || header.major != PROTOCOL_MAJOR
	    || header.minor != PROTOCOL_MINOR)
		return 0;
	return 1;
}

int
game_net_poll_move(void)
{
	return 1;
}

int
game_net_recv_move(struct move *move)
{
	struct move move_be;
	uint32_t val;

	val = OTRcv(conn_endpoint, (char *)&move_be, sizeof (move_be), 0);
	if (val == 0 || val == -1)
		return 0;
	move->location = ntohl(move_be.location);
	move->capture = ntohl(move_be.capture);
	move->promotion = ntohl(move_be.promotion);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			move->resulting_board[i][j] = ntohll(move_be.resulting_board[i][j]);
	return 1;
}

void
game_net_send_move(struct move *move)
{
	struct move move_be;
	move_be.location = htonl(move->location);
	move_be.capture = htonl(move->capture);
	move_be.promotion = htonl(move->promotion);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			move_be.resulting_board[i][j] = htonll(move->resulting_board[i][j]);
	OTSnd(conn_endpoint, (char *)&move_be, sizeof (move_be), 0);
}

void
game_net_disconnect(void)
{
	OTCloseProvider(conn_endpoint);
	conn_endpoint = NULL;
/* 	if (server_sock != -1) { */
/* 		close(server_sock); */
/* 		server_sock = -1; */
/* #ifdef USE_BONJOUR */
/* 		if (sd_register != NULL) */
/* 			DNSServiceRefDeallocate(sd_register); */
/* 		sd_register = NULL; */
/* #endif */
/* 	} */
}
