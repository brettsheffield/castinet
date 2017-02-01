/*
 * listener.c - multicast listener program
 *
 * this file is part of CASTINET
 *
 * Copyright (c) 2017 Brett Sheffield <brett@gladserv.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING in the distribution).
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "castinet.h"

#define default_group "ff3e::"
char **addrs = NULL;
int groups = 0;
char *port = "4242";
char *groupname = NULL;
char *src = NULL;

void exit_program(int ret)
{
	free(addrs);
        _exit(ret);
}

void print_usage(char *prog, int ret)
{
	printf("usage: %s [--addr multicast address] [--port port] [--grp groupname] [--src source address]\n", prog);
	_exit(ret);
}

void getaddrinfo_error(int e)
{
	printf("getaddrinfo error: %i\n", e);
}

void process_arg(int *i, char **argv)
{
	char *addr = NULL;

        if (strcmp(argv[*i], "--help") == 0) {
                print_usage(argv[0], 0);
        }
        else if (strcmp(argv[*i], "--addr") == 0) {
                addr = argv[++(*i)];
                addrs = realloc(addrs, sizeof(addrs) + sizeof(char *));
                addrs[groups++] = addr;
        }
        else if (strcmp(argv[*i], "--port") == 0) {
                port = argv[++(*i)];
        }
	else if (strcmp(argv[*i], "--grp") == 0) {
		groupname = argv[++(*i)];
	        printf("group: %s\n", groupname);
	}
        else if (strcmp(argv[*i], "--src") == 0) {
                src = argv[++(*i)];
        }
        else {
                print_usage(argv[0], 1);
        }
}

void process_args(int argc, char **argv)
{
        int i;

        if (argc > 1) {
                for (i = 1; i < argc; ++i) process_arg(&i, argv);
        }
}

void sig_handler(int signo)
{
        switch (signo) {
        case SIGINT:
                exit_program(0);
        default:
                break;
        }
}

int main(int argc, char **argv)
{
	int s_in;
	int l;
	int e;
	int g;
	struct addrinfo *castaddr = NULL;
	struct addrinfo *localaddr = NULL;
	struct addrinfo *srcaddr = NULL;
	struct addrinfo hints = {0};
	struct ipv6_mreq req;
	struct group_source_req grp;
	char buf[1024];
	char *addr;
	char txtaddr[INET6_ADDRSTRLEN];

	process_args(argc, argv);

	signal(SIGINT, sig_handler);

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;

	/* find local address to bind to */
	hints.ai_flags = AI_PASSIVE;
	if ((e = getaddrinfo(NULL, port, &hints, &localaddr)) != 0) {
		perror("getaddrinfo (in)");
		getaddrinfo_error(e);
		goto main_fail;
	}

	/* create inbound datagram socket */
	s_in = socket(localaddr->ai_family, localaddr->ai_socktype, 0);
	if (s_in == -1) {
		perror("socket (in)");
		goto main_fail;
	}

	if (bind(s_in, localaddr->ai_addr, localaddr->ai_addrlen) != 0) {
		perror("bind");
		goto main_fail;
	}

	if (src) {
		if ((e = getaddrinfo(src, port, &hints, &srcaddr)) != 0) {
			getaddrinfo_error(e);
			goto main_fail;
		}
	}

	if (groups == 0) {
		addr = default_group;
                addrs = realloc(addrs, sizeof(addrs) + sizeof(char *));
                addrs[groups++] = addr;
	}

	/* join multicast groups */
	for (g = 0; g < groups; g++) {
		addr = addrs[g];
		if (hashgroup(addr, groupname, txtaddr) != 0) {
			return 1;
		}
		if (groupname)
			addr = txtaddr;
		if ((e = getaddrinfo(addr, port, &hints, &castaddr)) != 0) {
			perror("getaddrinfo (out)");
			getaddrinfo_error(e);
			goto main_fail;
		}
		if (src) {
			printf("SSM mode (source: %s) joining %s\n", src, addr);
			memset(&grp, 0, sizeof(grp));
			memcpy(&grp.gsr_group,
					(struct sockaddr_in6*)(castaddr->ai_addr),
					sizeof(struct sockaddr_in6));
			memcpy(&grp.gsr_source,
					(struct sockaddr_in6*)(srcaddr->ai_addr),
					sizeof(struct sockaddr_in6));
			grp.gsr_interface = 0;
			if (setsockopt(s_in, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP,
				&grp, sizeof(grp)) == -1)
			{
				perror("multicast join");
				goto main_fail;
			}
		}
		else {
			printf("ASM mode join %s\n", addr);
			memcpy(&req.ipv6mr_multiaddr,
				&((struct sockaddr_in6*)(castaddr->ai_addr))->sin6_addr,
				sizeof(req.ipv6mr_multiaddr));

			req.ipv6mr_interface = 0; /* default interface */
			if (setsockopt(s_in, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
				&req, sizeof(req)) != 0)
			{
				perror("multicast join");
				goto main_fail;
			}
		}
		freeaddrinfo(castaddr);
	}

	freeaddrinfo(srcaddr);
	freeaddrinfo(localaddr);

	for (;;) {
		l = recvfrom(s_in, buf, sizeof(buf)-1, 0, NULL, NULL);
		buf[l] = '\0';
		printf("%s\n", buf);
	}

	/* not reached */

	return 0;

main_fail:
	return 1;
}
