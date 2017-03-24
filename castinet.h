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

char **addrs;
extern char program_usage[];

/* exit program with status */
void exit_program(int ret);

/* hash groupname and XOR with baseaddr to give hashaddr
 * return 0 on success */
int hashgroup(char *baseaddr, char *groupname, char *hashaddr);

/* print program usage and exit */
void print_usage(char *prog, int ret);

/* catch signals */
void sig_handler(int signo);

#endif /* __CASTINET_CASTINET_H__ */
