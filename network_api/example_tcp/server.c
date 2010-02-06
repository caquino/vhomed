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


#include <net.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void client (net_t *c)
{
	net_t net;
	char temp[128];

	memcpy (&net, c, sizeof (net_t));

	fprintf (stderr, "New client: %s\n", inet_ntoa (net.sock.sin_addr));

	memset (temp, 0, sizeof (temp));
	net_tcp_recv (&net, 0, temp, sizeof (temp));
	fprintf (stderr, "Message [%s]: %s\n",
		inet_ntoa (net.sock.sin_addr), temp);
	
	net_tcp_finish (&net);

	_exit (1);
}

int main (int argc, char **argv)
{
	net_t net, c;

	if (argc != 2) {
		fprintf (stderr, "usage: %s port\n", *argv);
		return 0;
	}

	signal (SIGCHLD, SIG_IGN);

	if (!net_tcp_server (&net, atoi (argv[1])))
		fprintf (stderr, "Can't create server.\n"), exit (1);
	
	for (;;) {
		if (!net_tcp_accept (&net, &c, 100)) continue;

		switch (fork ()) {
			case -1:
				perror ("fork");
				exit (1);
			case 0:
				client (&c);
				break;
			default:
				sleep (1);
				break;
		}
	}

	net_tcp_finish (&net);
	return 0;
}
