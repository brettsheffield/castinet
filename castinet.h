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

#ifndef __CASTINET_CASTINET_H__
#define __CASTINET_CASTINET_H__ 1

#ifndef __USE_KERNEL_IPV6_DEFS
struct in6_pktinfo
{
        struct in6_addr ipi6_addr;  /* src/dst IPv6 address */
        unsigned int ipi6_ifindex;  /* send/recv interface index */
};
#endif /* !__USE_KERNEL_IPV6_DEFS */

#define LOG_LEVELS(X) \
        X(0,    LOG_NONE,       "none")                                 \
        X(1,    LOG_SEVERE,     "severe")                               \
        X(2,    LOG_ERROR,      "error")                                \
        X(4,    LOG_WARNING,    "warning")                              \
        X(8,    LOG_INFO,       "info")                                 \
        X(16,   LOG_TRACE,      "trace")                                \
        X(32,   LOG_FULLTRACE,  "fulltrace")                            \
        X(64,   LOG_DEBUG,      "debug")
#undef X

#define LOG_ENUM(id, name, desc) name = id,
enum {
        LOG_LEVELS(LOG_ENUM)
};

char **addrs;
extern unsigned int loglevel;
extern char program_usage[];

/* exit program with status */
void exit_program(int ret);

/* hash groupname and XOR with baseaddr to give hashaddr
 * return 0 on success */
int hashgroup(char *baseaddr, char *groupname, char *hashaddr);

/* log message, if loglevel > level */
void logmsg(int level, char *msg, ...);

/* print program usage and exit */
void print_usage(char *prog, int ret);

/* catch signals */
void sig_handler(int signo);

/* return size of buffer to allocate for vsnprintf() */
int _vscprintf (const char * format, va_list argp);

#endif /* __CASTINET_CASTINET_H__ */
