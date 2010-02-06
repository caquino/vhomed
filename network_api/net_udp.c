/*
**  Network API.
**  Copyright (C) 2001 Alex Fiori <alex@linuxbr.com>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307
**  USA, or contact Alex Fiori <alex@linuxbr.com>
*/

#define __USE_GNU
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "net.h"

#ifndef vdprintf
extern int vdprintf (int fd, const char *fmt, va_list ap);
#endif

int net_udp_server (net_t *n, unsigned short int port)
{
	n->connected = 0;
	n->sock.sin_family      = AF_INET;
	n->sock.sin_port        = htons (port);
	n->sock.sin_addr.s_addr = INADDR_ANY;

	n->fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (n->fd < 0) return 0;

	if (bind (n->fd, (struct sockaddr *) &n->sock, sizeof (n->sock)) < 0)
		return 0;
	
	n->connected = 1;
	return 1;
}

int net_udp_client (net_t *n, unsigned short int port, const char *addr)
{
	n->connected = 0;
	n->sock.sin_family      = AF_INET;
	n->sock.sin_port        = htons (port);
	n->sock.sin_addr.s_addr = inet_addr (addr);

	n->fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (n->fd < 0) return 0;

	if (connect (n->fd, (struct sockaddr *) &n->sock, sizeof (n->sock)) < 0)
		return 0;
	else
		n->connected = 1;
	
	return 1;
}

int net_udp_send   (net_t *n, const char *fmt, ...)
{
	int r;
	va_list ap;

	if (!n->connected) return 0;

	va_start (ap, fmt);
	r = vdprintf (n->fd, fmt, ap);
	va_end (ap);

	return r <= 0 ? 0 : 1;
}

int net_udp_recv   (net_t *n, int timeout, struct sockaddr_in *from,
		    char *buffer, ssize_t lenght)
{
	int r;
	fd_set fds;
	int fromlen;
	struct timeval tv;

	if (!n->connected) return 0;

	FD_ZERO (&fds);
	FD_SET (n->fd, &fds);

	tv.tv_sec = 0;
	tv.tv_usec = timeout * 10000;

	if (select (n->fd + 1, &fds, NULL, NULL,
		!timeout ? NULL : &tv) <= 0) return 0;

	fromlen = sizeof (struct sockaddr_in);
	r = recvfrom (n->fd, buffer, lenght, 0,
		(struct sockaddr *) from, &fromlen);
	buffer[r] = '\0';

	return r;
}

void net_udp_finish (net_t *n)
{
	n->connected = 0;

	shutdown (n->fd, 2);
	close (n->fd);
}
