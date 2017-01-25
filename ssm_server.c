/* simple test of ipv6 SSM multicast */

#include <netdb.h>
#include <netinet/in.h>
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

void print_usage(char *prog, int ret)
{
	printf("usage: %s [--src source address] message\n", prog);
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

int main(int argc, char **argv)
{
	int s_out;
	int opt;
	struct addrinfo *castaddr = NULL;
	struct addrinfo hints = {0};

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

	free(msg);

	return 0;

main_fail:
	return 1;
}
