/* simple test of ipv6 SSM multicast */

#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *addr = "ff3e::1";
char *port = "4242";

void print_usage(char *prog, int ret)
{
	printf("usage: %s [--src <source address>]\n", prog);
	_exit(ret);
}

void getaddrinfo_error(int e)
{
	printf("getaddrinfo error: %i\n", e);
}

int main(int argc, char **argv)
{
	int s_in;
	int l;
	int e;
	struct addrinfo *castaddr = NULL;
	struct addrinfo *localaddr = NULL;
	struct addrinfo *srcaddr = NULL;
	struct addrinfo hints = {0};
	struct ipv6_mreq req;
	struct group_source_req grp;
	char buf[1024];
	char *src = NULL;

	/* check args */
	if (argc > 1) {
		if (argc == 3) {
			if (strcmp(argv[1], "--src") == 0) {
				src = argv[2];
			}
			else {
				print_usage(argv[0], 1);
			}
		}
		else {
			print_usage(argv[0], 1);
		}
	}

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;
	if ((e = getaddrinfo(addr, port, &hints, &castaddr)) != 0) {
		perror("getaddrinfo (out)");
		getaddrinfo_error(e);
		goto main_fail;
	}

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

	/* join multicast group */
	if (src) {
		printf("SSM mode (source: %s)\n", src);
		if ((e = getaddrinfo(src, port, &hints, &srcaddr)) != 0) {
			getaddrinfo_error(e);
			goto main_fail;
		}
		memset(&grp, 0, sizeof(grp));
		/* FIXME: use struct sockaddr */
		memcpy(&grp.gsr_group, castaddr, sizeof(struct sockaddr_in6));
		memcpy(&grp.gsr_source, srcaddr, sizeof(struct sockaddr_in6));
		grp.gsr_interface = 0;
		if (setsockopt(s_in, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP,
			&grp, sizeof(grp)) == -1)
		{
			perror("multicast join");
			goto main_fail;
		}
		freeaddrinfo(srcaddr);
	}
	else {
		printf("ASM modde\n");
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

	freeaddrinfo(localaddr);
	freeaddrinfo(castaddr);

	for (;;) {
		l = recvfrom(s_in, buf, sizeof(buf)-1, 0, NULL, NULL);
		buf[l] = '\0';
		printf("%s\n", buf);
	}

	return 0;

main_fail:
	return 1;
}
