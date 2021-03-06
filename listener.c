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

#include <arpa/inet.h>
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
#define BUFSIZE 1024
char **addrs = NULL;
char *relayaddr;
struct addrinfo *relaycast = NULL;
char program_usage[] = "usage: %s [--addr multicast address] [--port port] [--grp groupname] [--src source address]\n";
int groups = 0;
int relay = 0;
char *port = "4242";
char *groupname = NULL;
char *src = NULL;
unsigned int loglevel = 15;
long ttl = 9;

void exit_program(int ret)
{
	free(addrs);
	freeaddrinfo(relaycast);
	_exit(ret);
}

void getaddrinfo_error(int e)
{
	logmsg(LOG_ERROR, "getaddrinfo error: %i", e);
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
	else if (strcmp(argv[*i], "--debug") == 0) {
		loglevel = 127;
	}
	else if (strcmp(argv[*i], "--port") == 0) {
		port = argv[++(*i)];
	}
	else if (strcmp(argv[*i], "--grp") == 0) {
		groupname = argv[++(*i)];
		logmsg(LOG_INFO, "group: %s", groupname);
	}
	else if (strcmp(argv[*i], "--relay") == 0) {
		relay = 1;
		relayaddr = argv[++(*i)];
		logmsg(LOG_INFO, "relay address: %s", relayaddr);
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

int handle_msg(int sock, char *dstaddr, char *data, int data_len)
{
	struct iovec iov;
	struct msghdr msgh;
	char cmsgbuf[data_len];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	struct cmsghdr *cmsg;
	struct in6_pktinfo *pi;
	struct in6_addr da;
	int l;

	memset(&msgh, 0, sizeof(struct msghdr));
	iov.iov_base = data;
	iov.iov_len = data_len - 1;
	msgh.msg_control = cmsgbuf;
	msgh.msg_controllen = data_len - 1;
	msgh.msg_name = &from;
	msgh.msg_namelen = fromlen;
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_flags = 0;

	l = recvmsg(sock, &msgh, 0);

	if (l > 0) {
		dstaddr[0] = '\0';
		for (cmsg = CMSG_FIRSTHDR(&msgh);
		     cmsg != NULL;
		     cmsg = CMSG_NXTHDR(&msgh, cmsg))
		{
			if ((cmsg->cmsg_level == IPPROTO_IPV6)
			  && (cmsg->cmsg_type == IPV6_PKTINFO))
			{
				pi = (struct in6_pktinfo *) CMSG_DATA(cmsg);
				da = pi->ipi6_addr;
				inet_ntop(AF_INET6, &da, dstaddr, INET6_ADDRSTRLEN);
				break;
			}
		}
		data[l] = '\0';
	}

	if (relay) {
		logmsg(LOG_DEBUG, "relaying packet to %s", relayaddr);
		logmsg(LOG_DEBUG, "packet arrived on ifindex=%i", pi->ipi6_ifindex);
		sendto(sock, data, strlen(data), 0, relaycast->ai_addr,
				relaycast->ai_addrlen);
	}

	return l;
}

int getrelayaddrinfo(char *relayaddr, char *port)
{
	struct addrinfo hints = {0};
	if (getaddrinfo(relayaddr, port, &hints, &relaycast) != 0) {
		perror("getaddrinfo - relay addr");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int sock;
	int l;
	int e;
	int g;
	struct addrinfo *castaddr = NULL;
	struct addrinfo *localaddr = NULL;
	struct addrinfo *srcaddr = NULL;
	struct addrinfo hints = {0};
	struct ipv6_mreq req;
	struct group_source_req grp;
	char buf[BUFSIZE];
	char *addr;
	char txtaddr[INET6_ADDRSTRLEN];
	char dstaddr[INET6_ADDRSTRLEN];


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
	sock = socket(localaddr->ai_family, localaddr->ai_socktype, 0);
	if (sock == -1) {
		perror("socket (in)");
		goto main_fail;
	}

	if (relay) {
		logmsg(LOG_INFO, "relay mode enabled");

		if (getrelayaddrinfo(relayaddr, port) != 0) {
			logmsg(LOG_ERROR, "invalid --relay address");
			goto main_fail;
		}

		/* we're sending, so set TTL */
		l = ttl;
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &l,
					sizeof(l)) != 0)
		{
			perror("ttl");
			goto main_fail;
		}
	}

	if (bind(sock, localaddr->ai_addr, localaddr->ai_addrlen) != 0) {
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
			logmsg(LOG_INFO, "SSM mode (source: %s) joining %s", src, addr);
			memset(&grp, 0, sizeof(grp));
			memcpy(&grp.gsr_group,
					(struct sockaddr_in6*)(castaddr->ai_addr),
					sizeof(struct sockaddr_in6));
			memcpy(&grp.gsr_source,
					(struct sockaddr_in6*)(srcaddr->ai_addr),
					sizeof(struct sockaddr_in6));
			grp.gsr_interface = 0;
			if (setsockopt(sock, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP,
				&grp, sizeof(grp)) == -1)
			{
				perror("multicast join");
				goto main_fail;
			}
		}
		else {
			logmsg(LOG_INFO, "ASM mode join %s", addr);
			memcpy(&req.ipv6mr_multiaddr,
				&((struct sockaddr_in6*)(castaddr->ai_addr))->sin6_addr,
				sizeof(req.ipv6mr_multiaddr));

			req.ipv6mr_interface = 0; /* default interface */
			if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
				&req, sizeof(req)) != 0)
			{
				perror("multicast join");
				goto main_fail;
			}
		}
		freeaddrinfo(castaddr);
	}

	/* request ancilliary control data */
	l = 1;
	setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &l, sizeof(l));

	freeaddrinfo(srcaddr);
	freeaddrinfo(localaddr);

	for (;;) {
		if (handle_msg(sock, dstaddr, buf, BUFSIZE) > 0) {
			logmsg(LOG_INFO, "[%s] %s", dstaddr, buf);
		}
	}

	/* not reached */

	return 0;

main_fail:
	return 1;
}
