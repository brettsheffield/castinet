# Makefile for castinet
#
# this file is part of CASTINET
#
# Copyright (c) 2017 Brett Sheffield <brett@gladserv.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (see the file COPYING in the distribution).
# If not, see <http://www.gnu.org/licenses/>.
 
COPTS=-Wall -Werror -g

all: listener castinet

listener: listener.c
	gcc ${COPTS} -o listener listener.c

castinet: source.c
	gcc ${COPTS} -o castinet source.c

.PHONY: clean

clean:
	rm listener castinet
