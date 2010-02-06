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

#ifndef NET_H
#define NET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

typedef struct {
	int fd;
	int connected;
	struct sockaddr_in sock;
	struct sockaddr_un domain;
} net_t;

/* UDP */
extern int net_udp_server (net_t *n, unsigned short int port);
extern int net_udp_client (net_t *n, unsigned short int port, const char *addr);
extern int net_udp_send   (net_t *n, const char *fmt, ...);
extern int net_udp_recv   (net_t *n, int timeout, struct sockaddr_in *from, 
	  					   char *buffer, ssize_t lenght);
extern void net_udp_finish (net_t *n);

/* TCP */
extern int net_tcp_server (net_t *n, unsigned short int port);
extern int net_tcp_client (net_t *n, unsigned short int port, const char *addr);
extern int net_tcp_accept (net_t *n, net_t *c, int timeout);
extern int net_tcp_send   (net_t *c, const char *fmt, ...);
extern int net_tcp_recv   (net_t *c, int timeout, char *buffer, ssize_t lenght);
extern void net_tcp_finish (net_t *n);

/* DOMAIN */
extern int net_domain_server (net_t *n, const char *sock);
extern int net_domain_client (net_t *n, const char *sock);
extern int net_domain_accept (net_t *n, net_t *c, int timeout);
extern int net_domain_send   (net_t *c, const char *fmt, ...);
extern int net_domain_recv   (net_t *c, int timeout, char *buffer, ssize_t lenght);
extern void net_domain_finish (net_t *n);


#endif /* net.h */
