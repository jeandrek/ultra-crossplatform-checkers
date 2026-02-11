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

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#if defined(__unix__) || defined(__APPLE__)
#include <net/if.h>
#include <ifaddrs.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include "config.h"

#ifdef USE_BONJOUR
#include <dns_sd.h>
#endif

#include "net_menu.h"
#include "game_net.h"

#ifndef _WIN32
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/* LE */
static inline uint64_t
byte_swap_64(uint64_t x)
{
	return (uint64_t)ntohl(x & 0xffffffff) << 32 | ntohl(x >> 32);
}

#define ntohll(x) byte_swap_64(x)
#define htonll(x) byte_swap_64(x)
#else
#define ntohll(x) x
#define htonll(x) x
#endif
#endif

static int	game_net_connect_to_client(void);
static void	game_net_send_header(void);
static int	game_net_check_header(void);

static int	server_sock = -1;
static int	conn_sock = -1;
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
#ifdef __psp__
	return "";
#else
	struct in_addr addr = {0};
#if defined(_WIN32)
	struct addrinfo *info;
	struct addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo("", NULL, &hints, &info);
	if (info != NULL) {
		struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
		addr = sa->sin_addr;
	}
	freeaddrinfo(info);
#elif defined(__unix__) || defined(__APPLE__)
	struct ifaddrs *ifaddr;

	getifaddrs(&ifaddr);
	for (struct ifaddrs *cur = ifaddr; cur != NULL; cur = cur->ifa_next) {
		if (cur->ifa_addr->sa_family == AF_INET) {
			if (!(cur->ifa_flags & IFF_LOOPBACK)) {
				struct sockaddr_in *sa =
					(struct sockaddr_in *)cur->ifa_addr;
				addr = sa->sin_addr;
				break;
			}
		}
	}
	freeifaddrs(ifaddr);
#endif
	return inet_ntoa(addr);
#endif
}

int
game_net_connected(void)
{
	return conn_sock != -1;
}

int
game_net_host(int player)
{
	struct sockaddr_in sa;
	int reuse;
	int sock;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(7440);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	reuse = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		   (char *)&reuse, sizeof (reuse));

	if (bind(sock, (struct sockaddr *)&sa, sizeof (sa)) < 0)
		return 0;
	listen(sock, 1);
	server_sock = sock;
	game_net_player = player;

#ifdef USE_BONJOUR
	DNSServiceErrorType err;

	err = DNSServiceRegister(&sd_register, 0, 0, NULL, "_checkers._tcp",
				 NULL, NULL, htons(7440), 0, NULL,
				 NULL, NULL);
	if (!err)
		DNSServiceProcessResult(sd_register);
#endif

	return 1;
}

/*
 * Alternatively could use another thread and blocking operations.
 */

int
game_net_poll_connections(void)
{
	struct timeval timeout = {0, 0};
	int result;

	if (server_sock == -1)
		return 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(server_sock, &fds);
	result = select(server_sock + 1, &fds, NULL, NULL, &timeout);
	if (!result)
		return 0;
	if (!game_net_connect_to_client())
		return 0;
	return 1;
}

static int
game_net_connect_to_client(void)
{
	conn_sock = accept(server_sock, NULL, NULL);
	game_net_send_header();
	if (!game_net_check_header())
		goto bad_conn;
	send(conn_sock, &game_net_player, 1, 0);
	return 1;

bad_conn:
	close(conn_sock);
	conn_sock = -1;
	return 0;
}

void
game_net_stop_hosting(void)
{
	close(server_sock);
#ifdef USE_BONJOUR
	if (sd_register != NULL)
		DNSServiceRefDeallocate(sd_register);
#endif
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

void
game_net_join(const char *nodename)
{
	struct sockaddr_in sa;
	struct hostent *hostent = gethostbyname(nodename);
	int result;

	conn_sock_state = CANT_CONNECT;
	if (hostent == NULL)
		return;
	sa.sin_family = AF_INET;
	sa.sin_port = ntohs(7440);
	sa.sin_addr = *(struct in_addr *)hostent->h_addr_list[0];

	conn_sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
	unsigned long nonblocking = 1;
	ioctlsocket(conn_sock, FIONBIO, &nonblocking);
#else
	fcntl(conn_sock, F_SETFL, O_NONBLOCK);
#endif
	result = connect(conn_sock, (struct sockaddr *)&sa, sizeof (sa));
#ifdef _WIN32
	if (result < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
#else
	if (result < 0 && errno != EINPROGRESS && errno != EWOULDBLOCK)
#endif
		return;
	conn_sock_state = CONNECTING;
}

int
game_net_poll_connected(void)
{
	struct timeval timeout = {0, 0};
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(conn_sock, &fds);
	if (conn_sock_state == CANT_CONNECT) {
		return -1;
	} if (conn_sock_state == CONNECTING) {
		if (select(conn_sock + 1, NULL, &fds, NULL, &timeout) > 0) {
#ifndef _WIN32
			int err;
			socklen_t len = sizeof (err);
			getsockopt(conn_sock, SOL_SOCKET, SO_ERROR,
				   (char *)&err, &len);
			if (err)
				goto bad_conn;
#endif
			conn_sock_state = WAITING_FOR_HEADER;
			game_net_send_header();
			return 0;
		}
#ifdef _WIN32
		FD_ZERO(&fds);
		FD_SET(conn_sock, &fds);
		if (select(conn_sock + 1, NULL, NULL, &fds, &timeout) > 0)
			goto bad_conn;
#endif
	} else if (conn_sock_state == WAITING_FOR_HEADER) {
		if (select(conn_sock + 1, &fds, NULL, NULL, &timeout) > 0) {
			if (!game_net_check_header())
				goto bad_conn;
			conn_sock_state = WAITING_FOR_OPPONENT;
			return 0;
		}
	} else if (conn_sock_state == WAITING_FOR_OPPONENT) {
		char opponent;
		if (select(conn_sock + 1, &fds, NULL, NULL, &timeout) > 0) {
			if (recv(conn_sock, &opponent, 1, 0) < 1)
				goto bad_conn;
			game_net_player = !opponent;
			return 1;
		}
	}
	return 0;
bad_conn:
	close(conn_sock);
	conn_sock = -1;
	return -1;
}

void
game_net_stop_connecting(void)
{
	close(conn_sock);
	conn_sock = -1;
}

static void
game_net_send_header(void)
{
	struct checkers_header header;

	header.magic[0] = 0xff;
	header.magic[1] = 'C';
	header.major = PROTOCOL_MAJOR;
	header.minor = PROTOCOL_MINOR;
	send(conn_sock, (char *)&header, sizeof (header), 0);
}

static int
game_net_check_header(void)
{
	struct checkers_header header;

	if (recv(conn_sock, (char *)&header, sizeof (header), 0)
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
	struct timeval timeout = {0, 0};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(conn_sock, &fds);
	return select(conn_sock + 1, &fds, NULL, NULL, &timeout);
}

int
game_net_recv_move(struct move *move)
{
	struct move move_be;
	ssize_t val;

	val = recv(conn_sock, (char *)&move_be, sizeof (move_be), 0);
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
	send(conn_sock, (char *)&move_be, sizeof (move_be), 0);
}

void
game_net_disconnect(void)
{
	close(conn_sock);
	conn_sock = -1;
	if (server_sock != -1) {
		close(server_sock);
		server_sock = -1;
#ifdef USE_BONJOUR
		if (sd_register != NULL)
			DNSServiceRefDeallocate(sd_register);
		sd_register = NULL;
#endif
	}
}
