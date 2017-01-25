/*
 * listener.c - multicast sender program
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

#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *addr = "ff3e::1";
char *port = "4242";
char *msg = NULL;
char *tmp;
int ttl = 9;
int loop = 1;

void exit_program(int ret)
{
	free(msg);
	_exit(ret);
}

void print_usage(char *prog, int ret)
{
	printf("usage: %s [--addr multicast address] [--port multicast port] message\n", prog);
	_exit(ret);
}

void process_arg(int *i, char **argv)
{
	if (strcmp(argv[*i], "--help") == 0) {
		print_usage(argv[0], 0);
	}
	else if (strcmp(argv[*i], "--addr") == 0) {
		addr = argv[++(*i)];
	}
	else if (strcmp(argv[*i], "--port") == 0) {
		port = argv[++(*i)];
	}
	else {
		print_usage(argv[0], 1);
	}
}

void process_args(int argc, char **argv)
{
	int i;

	if (argc > 1) {
		for (i = 1; i < argc; ++i) {
			if (strncmp(argv[i], "--", 2) == 0) {
				if (i == argc)
					print_usage(argv[0], 1);
				process_arg(&i, argv);
			}
			else {
				int j = 0;
				int l = 0;
				for (j = i; j < argc; j++) l += strlen(argv[j]) + 1;
				msg = malloc(l);
				while (i < argc) {
					strcat(msg, argv[i++]);
					strcat(msg, " ");
				}
				msg[l-1] = '\0';
				printf("msg: %s\n", msg);
			}
		}
	}
	if (msg == NULL) {
		print_usage(argv[0], 1);
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
	int s_out;
	int opt;
	struct addrinfo *castaddr = NULL;
	struct addrinfo hints = {0};

	signal(SIGINT, sig_handler);

	process_args(argc, argv);

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(addr, port, &hints, &castaddr) != 0) {
		perror("getaddrinfo (out)");
		goto main_fail;
	}

	/* create outbound datagram socket */
	s_out = socket(castaddr->ai_family, castaddr->ai_socktype, 0);
	if (s_out == -1) {
		perror("socket (out)");
		goto main_fail;
	}

	/* set TTL */
	opt = ttl;
	if (setsockopt(s_out, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &opt,
				sizeof(opt)) != 0)
	{
		perror("ttl");
		goto main_fail;
	}

	/* set loopback */
	opt = loop;
	if (setsockopt(s_out, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &opt,
				sizeof(opt)) != 0)
	{
		perror("loop");
		goto main_fail;
	}

	freeaddrinfo(castaddr);

	for (;;) {
		sendto(s_out, msg, strlen(msg), 0, castaddr->ai_addr,
				castaddr->ai_addrlen);
		sleep(1);
	}

	/* not reached */

	return 0;

main_fail:
	return 1;
}
