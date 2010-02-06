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

	if (argc != 3) {
		fprintf (stderr, "usage: %s sock message\n", *argv);
		return 0;
	}

	if (!net_domain_client (&net, argv[1]))
		fprintf (stderr, "Can't connect.\n"), exit (1);
	
	net_domain_send (&net, argv[2], strlen (argv[2]));
	net_domain_finish (&net);

	return 0;
}
