/*
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
#include <assert.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "castinet.h"

int hashgroup(char *baseaddr, char *groupname, char *hashaddr)
{
	int i;
	unsigned char hashgrp[SHA_DIGEST_LENGTH];
	unsigned char binaddr[16];

	if (groupname) {
		SHA1((unsigned char *)groupname, strlen(groupname), hashgrp);

		if (inet_pton(AF_INET6, baseaddr, &binaddr) != 1) {
			logmsg(LOG_ERROR, "invalid address");
			return 1;
		}

		/* we have 112 bits (14 bytes) available for the group address
		 * XOR the hashed group with the base multicast address */
		for (i = 0; i < 14; i++) {
			binaddr[i+2] ^= hashgrp[i];
		}

		inet_ntop(AF_INET6, binaddr, hashaddr, INET6_ADDRSTRLEN);
	}

	return 0;
}

void logmsg(int level, char *msg, ...)
{
        va_list argp;
        char *b;

        if ((loglevel & level) != level)
                return;

        va_start(argp, msg);
        b = malloc(_vscprintf(msg, argp) + 1);
        assert(b != NULL);
        vsprintf(b, msg, argp);
        va_end(argp);
        fprintf(stderr, "%s\n", b);
        free(b);
}

void print_usage(char *prog, int ret)
{
        logmsg(LOG_ERROR, "%s", program_usage);
        _exit(ret);
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

int _vscprintf (const char * format, va_list argp)
{
        int r;
        va_list argc;
        va_copy(argc, argp);
        r = vsnprintf(NULL, 0, format, argc);
        va_end(argc);
        return r;
}
