/*
**  vhomed
**  Copyright (C) 2001 Cassiano Aquino <cassiano@wwsecurity.net>
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
**  USA, or contact Cassiano Aquino <cassiano@wwsecurity.net>
**
**  $Id$
*/

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <mysql/mysql.h>
#include "network_api/net.h"
#include "vhome.h"

MYSQL db;

char *timestamp()
{
  char timevar[100];
  time_t lt;

  lt = time(NULL);
  strftime(timevar,sizeof(timevar) - 1,"%a, %d %b %Y %T GMT",localtime(&lt));
  timevar[sizeof(timevar)-1]=0;
  return(char *)strdup(timevar);
}

void client (net_t *c)
{
    net_t net;
    char temp[128], query[MAXLEN], *type, *what;
	int have = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

    memcpy (&net, c, sizeof (net_t));

    memset (temp, 0, sizeof (temp));
    memset (query,0, sizeof (query));

    net_domain_recv (&net, 0, temp, sizeof (temp));

    type = strtok(temp,":");
    what = strtok(NULL,":");

    if(mysql_ping(&db))
       fprintf(stderr,"[%s]: Conexao com o banco encerrada, reconectando\n", timestamp());
	switch (atoi(type)) {
	   	case 0:
			snprintf(query,MAXLEN,"SELECT %s FROM %s WHERE (STRCMP(%s,'#%s') = 0)",CHANURL, CHANTBL, CHANCMP, what);
			have++;
			break;
		case 1:
			snprintf(query,MAXLEN,"SELECT %s FROM %s WHERE %s = '%s'", USRURL, USRTBL, USRCMP, what);
			have++;
			break;
		default:
			fprintf(stderr,"[%s]: Table (%s) does not exist!\n",timestamp(),type);
			break;
	}
	if (have) {
		if (mysql_query(&db,query)) {
			fprintf(stderr,"[%s]: Query (%s) Status (%s)\n",timestamp(),query,mysql_error(&db));
			have--;
		} else {
			result = mysql_store_result(&db);
			if (!mysql_num_rows(result) || result == NULL || !mysql_num_fields(result)) {
				fprintf(stderr,"[%s]: Query (%s) Return (%s)\n",timestamp(),query,"Not Found");
				have--;
			} else {
				if ((row = mysql_fetch_row(result))) {
					if ((char *)row[0] != NULL) {
						fprintf(stderr,"[%s]: Query (%s) Return (%s)\n",timestamp(),query,(char *)row[0]);
					} else {
						fprintf(stderr,"[%s]: Query (%s) Return (%s)\n",timestamp(),query,"Not Found");
						have--;
					}
				}
			}
		}
	}
	if (!net_domain_send (&net, (have) ? (char *)row[0] : "0", strlen((have) ? (char *)row[0] : "0"))) {
		fprintf(stderr,"[%s]: Cannot send to socket (%s)\n",timestamp(), (have) ? (char *)row[0] : "Not Found");
	}
	if (have) 
		mysql_free_result(result);
    net_domain_finish (&net);
    _exit (1);
}

void usage(void)
{
   fprintf(stderr, "vhomed usage: vhomed -k (to kill vhomed).\n");
}

int main (int argc, char **argv)
{
    net_t net, c;
	int tty, fp, fpid, pid, size, t;
	char buffer[128];
	extern char * optarg;
	extern int opterr, optopt;

    signal (SIGCHLD, SIG_IGN);
    fprintf(stderr,"+ vhomed %s Cassiano Aquino <cassiano@wwsecurity.net>\n",VERSION);

	while ((t=getopt(argc,argv,":k")) != EOF) {
	   switch (t) {
		  case 'k':
			if ((fpid = open( PID, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR))) {
				if((size = read( fpid, buffer, sizeof(buffer)))) {
				   pid = atoi(buffer);
				   fprintf(stderr,"+ Checking PID(%d)\n",pid);
				   if (kill(pid, SIGKILL))
					  fprintf(stderr,"+ scooby-doo where are you?\n");
				   else
					  unlink(SOCKET);
				   	  unlink(PID);
					  fprintf(stderr,"+ bye, killing vhomed\n"), exit(1);
				}
				close(fpid);
			}
			return EXIT_SUCCESS;
			break;
		  case '?':
			fprintf(stderr,"Unknow option '-%c'.\n",optopt);
			usage();
			return EXIT_FAILURE;
			break;
	   }
	}
	if ((fpid = open( PID, O_RDONLY | O_CREAT , S_IRUSR | S_IWUSR))) {
		if((size = read( fpid, buffer, sizeof(buffer)))) {
		   pid = atoi(buffer);
		   fprintf(stderr,"+ Checking PID(%d)\n",pid);
		   if (kill(pid, SIGCONT))
			  fprintf(stderr,"+ Starting vhomed\n");
		   else
			  fprintf(stderr,"+ vhomed is running, bye\n"), exit(1);
		}
		close(fpid);
	}
    if (getuid() == 0) {
      fprintf(stderr,"if (getuid() == 0) exit(1);!\n");
      exit(1);
    }
    fprintf(stderr,"+ Opening db connection : ");
    mysql_init(&db);
    mysql_options(&db,MYSQL_OPT_COMPRESS,0);
    mysql_options(&db,MYSQL_READ_DEFAULT_GROUP,"client");
    if (!mysql_real_connect (&db, HOST, USER, PASS, DB,0,NULL,0))  {
      fprintf(stderr,"ERR : %s\n", mysql_error(&db));
      exit(1);
    } else {
      fprintf(stderr,"OK ");
    }
    fprintf(stderr,"Server version : %s\n",mysql_get_server_info(&db));
    unlink(SOCKET);

    if (!net_domain_server (&net, SOCKET))
        fprintf (stderr, "Can't create server.\n"), exit (1);

	switch (fork ()) {
		case -1:
			perror("fork");
			exit(1);
			break;
		case 0:
			fprintf(stderr,"+ PID : %d\n",getpid());
			snprintf(buffer, sizeof(buffer) - 1, "%d", getpid());
			if (! (fpid = open( PID, O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR )))
			  perror("open"), exit(1);
			if(! (write( fpid, buffer, strlen(buffer))))
			  perror("write"), exit(1);
			close(fpid);
			tty = open("/dev/tty",  O_WRONLY);
			ioctl(tty,TIOCNOTTY,0);
			close(tty); umask(022); chdir("/tmp");
			if (!(fp = open(LOG,O_WRONLY | O_APPEND | O_CREAT , S_IRUSR | S_IWUSR )))
			perror("open"), exit(1);
			close(0); close(1); close(2);
			dup2(fp,0); dup2(fp,1); dup2(fp,2);
		    for (;;) {
		        if (!net_domain_accept (&net, &c, 100)) continue;
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

		    net_domain_finish (&net);
		    return 0;
			break;
	}

	exit(0);
}

/*
** $Log$
** EOF
*/
