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
#elif defined(__unix__)
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
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
