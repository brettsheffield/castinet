/* simple test of ipv6 SSM multicast */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

char *addr = "ff3e::1";
char *port = "4242";

int main()
{
	int s_in;
	int l;
	struct addrinfo *castaddr = NULL;
	struct addrinfo *localaddr = NULL;
	struct addrinfo hints = {0};
	struct ipv6_mreq req;
	char buf[1024];

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(addr, port, &hints, &castaddr) != 0) {
		perror("getaddrinfo (out)");
		goto main_fail;
	}

	/* find local address to bind to */
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, port, &hints, &localaddr) != 0) {
		perror("getaddrinfo (in)");
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


	memcpy(&req.ipv6mr_multiaddr,
			&((struct sockaddr_in6*)(castaddr->ai_addr))->sin6_addr,
			sizeof(req.ipv6mr_multiaddr));
	req.ipv6mr_interface = 0; /* default interface */

	/* join multicast group */
	if (setsockopt(s_in, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&req,
			sizeof(req)) != 0)
	{
		perror("multicast join");
		goto main_fail;
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
