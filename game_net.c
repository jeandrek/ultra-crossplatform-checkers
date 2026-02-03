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

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>

#define close closesocket
#elif defined(__unix__)
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif

#include "game_net.h"

#if !defined(_WIN32) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#include <byteswap.h>
/* LE */
#define ntohll bswap_64
#define htonll bswap_64
#else
#define ntohll(x) x
#define htonll(x) x
#endif

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
#elif defined(__unix__)
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

static int server_sock = -1;
static int conn_sock = -1;
int game_net_player = -1;

int
game_net_connected(void)
{
	return conn_sock != -1;
}

int
game_net_host(int player)
{
	struct sockaddr_in sa;
	int sock;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(7440);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(sock, (struct sockaddr *)&sa, sizeof (sa)) < 0)
		return 0;
	listen(sock, 1);
	server_sock = sock;
	game_net_player = player;
	return 1;
}

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
	conn_sock = accept(server_sock, NULL, NULL);
	return 1;
}

void
game_net_stop_hosting(void)
{
	close(server_sock);
}

int
game_net_join(char *addr)
{
	struct sockaddr_in sa;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(7440);
	sa.sin_addr.s_addr = inet_addr(addr);
	conn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(conn_sock, (struct sockaddr *)&sa, sizeof (sa)) < 0)
		return 0;
	game_net_player = 1;	/* XXX */
	return 1;
}

int
game_net_poll_move(void)
{
	struct timeval timeout = {0, 0};

	if (conn_sock == -1)
		return 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(conn_sock, &fds);
	return select(conn_sock + 1, &fds, NULL, NULL, &timeout);
}

void
game_net_recv_move(struct move *move)
{
	struct move move_be;
	recv(conn_sock, (char *)&move_be, sizeof (move_be), 0);
	move->location = ntohl(move_be.location);
	move->capture = ntohl(move_be.capture);
	move->promotion = ntohl(move_be.promotion);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			move->resulting_board[i][j] = ntohll(move_be.resulting_board[i][j]);
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
	if (server_sock != -1)
		close(server_sock);
}
