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
LIBS=-lcrypto

all: listener castinet

listener: listener.o castinet.o
	gcc ${COPTS} -o listener listener.o castinet.o ${LIBS}

castinet: source.o castinet.o
	gcc ${COPTS} -o castinet source.o castinet.o ${LIBS}

listener.o: listener.c
	gcc ${COPTS} -c listener.c ${LIBS}

source.o: source.c
	gcc ${COPTS} -c source.c ${LIBS}

castinet.o: castinet.c castinet.h
	gcc ${COPTS} -c castinet.c ${LIBS}

.PHONY: clean

clean:
	rm listener castinet *.o
