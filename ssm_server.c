/* simple test of ipv6 SSM multicast */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char *addr = "ff3e::1";
char *port = "4242";

int main()
{
	int s_out;
	int opt;
	struct addrinfo *castaddr = NULL;
	struct addrinfo hints = {0};
	char msg[] = "Barney was here";

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
	opt = 1;
	if (setsockopt(s_out, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &opt,
				sizeof(opt)) != 0)
	{
		perror("ttl");
		goto main_fail;
	}

	/* set loopback */
	opt = 1;
	if (setsockopt(s_out, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &opt,
				sizeof(opt)) != 0)
	{
		perror("loop");
		goto main_fail;
	}

	freeaddrinfo(castaddr);

	for (;;) {
		sendto(s_out, msg, sizeof(msg), 0, castaddr->ai_addr,
				castaddr->ai_addrlen);
		sleep(1);
	}

	return 0;

main_fail:
	return 1;
}
