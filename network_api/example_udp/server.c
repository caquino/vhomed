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
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
	net_t net;
	char temp[128];
	struct sockaddr_in s;

	if (argc != 2) {
		fprintf (stderr, "usage: %s port\n", *argv);
		return 0;
	}

	if (!net_udp_server (&net, atoi (argv[1])))
		fprintf (stderr, "Can't create server.\n"), exit (1);
	
	for (;;) {
		memset (temp, 0, sizeof (temp));
		if (net_udp_recv (&net, 100, &s, temp, sizeof (temp)) <= 0)
			continue;

		fprintf (stderr, "Message from %s: %s\n",
			 inet_ntoa (s.sin_addr), temp);
	}

	net_udp_finish (&net);
	return 0;
}
