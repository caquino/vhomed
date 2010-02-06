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

int net_tcp_server (net_t *n, unsigned short int port)
{
	n->connected = 0;
	n->sock.sin_family      = AF_INET;
	n->sock.sin_port        = htons (port);
	n->sock.sin_addr.s_addr = INADDR_ANY;

	n->fd = socket (AF_INET, SOCK_STREAM, 0);
	if (n->fd < 0) return 0;

	if (bind (n->fd, (struct sockaddr *) &n->sock, sizeof (n->sock)) < 0)
		return 0;
	
	if (listen (n->fd, 5) < 0) return 0;

	n->connected = 1;
	return 1;
}

int net_tcp_client (net_t *n, unsigned short int port, const char *addr)
{
	n->connected = 0;
	n->sock.sin_family      = AF_INET;
	n->sock.sin_port        = htons (port);
	n->sock.sin_addr.s_addr = inet_addr (addr);

	n->fd = socket (AF_INET, SOCK_STREAM, 0);
	if (n->fd < 0) return 0;

	if (connect (n->fd, (struct sockaddr *) &n->sock, sizeof (n->sock)) < 0)
		return 0;
	else
		n->connected = 1;

	return 1;
}

int net_tcp_accept (net_t *n, net_t *c, int timeout)
{
	int len;
	fd_set fds;
	struct timeval tv;
	
	c->connected = 0;
	if (!n->connected) return 0;

	FD_ZERO (&fds);
	FD_SET (n->fd, &fds);

	tv.tv_sec = 0;
	tv.tv_usec = timeout * 10000;

	if (select (n->fd + 1, &fds, NULL, NULL,
		!timeout ? NULL : &tv) <= 0) return 0;

	len = sizeof (struct sockaddr_in);
	c->fd = accept (n->fd, (struct sockaddr *) &c->sock, &len);
	if (c->fd < 0) return -1;

	c->connected = 1;

	return 1;
}

int net_tcp_send   (net_t *c, const char *fmt, ...)
{
	int r;
	va_list ap;

	if (!c->connected) return 0;

	va_start (ap, fmt);
	r = vdprintf (c->fd, fmt, ap);
	va_end (ap);

	return r <= 0 ? 0 : 1;
}

int net_tcp_recv   (net_t *c, int timeout, char *buffer, ssize_t lenght)
{
	int r;
	fd_set fds;
	struct timeval tv;

	if (!c->connected) return 0;

	FD_ZERO (&fds);
	FD_SET (c->fd, &fds);

	tv.tv_sec = 0;
	tv.tv_usec = timeout * 10000;

	if (select (c->fd + 1, &fds, NULL, NULL,
		!timeout ? NULL : &tv) <= 0) return 0;

	r = recv (c->fd, buffer, lenght, 0);
	buffer[r] = '\0';

	return r;
}

void net_tcp_finish (net_t *n)
{
	n->connected = 0;

	shutdown (n->fd, 2);
	close (n->fd);
}
