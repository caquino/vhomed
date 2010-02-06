/*
**  mod_vhome.
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

/* #include "general.h" */
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "ap_config.h"
#include "ap_compat.h"
#include "network_api/net.h"


#include "http_core.h"
#include "vhome.h"


module vhome_module;

/* Config Struct */

typedef struct {
  char *vhome_Table;
  char *vhome_TopURL;
  char *vhome_PromoURL;
} vhome_config_rec;


/* vhome_create_dir_config */
static void *vhome_create_dir_config(pool *p, char *d)
{
  vhome_config_rec *dcfg =
    (vhome_config_rec *) 
    ap_pcalloc(p, sizeof(vhome_config_rec));
  return dcfg;
}

static void vhome_init(server_rec *s, pool *p)
{
  ap_add_version_component("mod_vhome/2.2");
}


static char *vhome_get_user_home(vhome_config_rec *dcfg, char *what)
{
  char query[200];  
  int table;
  net_t net;

  if (!net_domain_client (&net, SOCKET))
	return NULL;

  memset (query, 0, sizeof(query));

  if (strcmp(dcfg->vhome_Table,"chan"))
	 table = 0;
  else if (strcmp(dcfg->vhome_Table,"users"))
	 table = 1;

  snprintf(query,200,"%d:%s",table,what);

  net_domain_send (&net, query, strlen (query));
  net_domain_recv (&net, 0, query, sizeof (query));
  net_domain_finish (&net);

  return  ((strstr(query,"0") == NULL) ? NULL : query);
}

static int vhome_transaction(request_rec *r, vhome_config_rec *dcfg) {
  char *user;

  char *p, *url = NULL;
  if ((strlen(r->uri) > strlen(dcfg->vhome_TopURL)) &&
      (0 == strncmp(dcfg->vhome_TopURL, 
		    r->uri, strlen(dcfg->vhome_TopURL)))) {
    user = r->uri + strlen(dcfg->vhome_TopURL);

    if (url = vhome_get_user_home(dcfg, user)) {
      url = ap_pstrdup(r->pool, url);
    } else {
	  ap_table_setn(r->headers_out, "X-vhomed-author","cassiano@wwsecurity.net");
	  ap_table_setn(r->headers_out, "X-powered-by","mod_vhomed by chaosmaker");
	  ap_table_setn(r->headers_out, "Location", "http://www.brasnet.org/");
      return 2;
    }

    /* Now, redirect or send framed html */
    if (dcfg->vhome_PromoURL) {
      r->content_type = "text/html";
      ap_send_http_header(r);
      /*
       * HTML Text
       */
      ap_rprintf(r, "<HTML>\n");
      ap_rprintf(r, "<HEAD>\n");
      ap_rprintf(r, "<BASE target=\"_top\">\n");
      ap_rprintf(r, "\n");
      ap_rprintf(r, "<FRAMESET rows=\"0,*\" marginwidth=\"0\" marginheight=\"0\" framespacing=0 frameborder=no border=0>\n");
      ap_rprintf(r, "<FRAME marginwidth=\"0\" marginheight=\"0\" src=\"%s\" name=pb scrollbars=no scrolling=no framespacing=0 frameborder=no border=0>\n",
		 dcfg->vhome_PromoURL);
      ap_rprintf(r, "<FRAME marginwidth=\"5\" marginheight=\"2\" src=\"%s\" name=thePage framespacing=0 frameborder=no border=0>\n", url);
      ap_rprintf(r, "</FRAMESET>\n");
      ap_rprintf(r, "\n");
      ap_rprintf(r, "<NOFRAMES>\n");
      ap_rprintf(r, "<h1 align=center>Frame ALERT!</h1>\n");
      ap_rprintf(r, "<p>\n");
      ap_rprintf(r, "This document is designed to be viewed using\n");
      ap_rprintf(r, "Frame features. If you are seeing this message, please consider\n");
      ap_rprintf(r, "upgrading to a frames-compatible browser.\n");
      ap_rprintf(r, "</p>\n");
      ap_rprintf(r, "\n");
      ap_rprintf(r, "</NOFRAMES>\n");
      ap_rprintf(r, "</HTML>\n");
      
      return 1;
    } else {
      ap_table_setn(r->headers_out, "Location", url);
      return 2;
    }
  } else {
	ap_table_setn(r->headers_out, "Location", "http://www.brasnet.org/");
    return 2;
  }
}

/* The vhome content handler */

static int vhome_handler(request_rec *r)
{
  vhome_config_rec *dcfg = (vhome_config_rec *) 
    ap_get_module_config(r->per_dir_config, &vhome_module);

  if (dcfg && dcfg->vhome_TopURL && dcfg->vhome_Table) {
    char *url;
	
    switch (vhome_transaction(r, dcfg)) {
    case 0: 
      return HTTP_NOT_FOUND;
    case 1: 
      return OK;
    case 2: 
      return REDIRECT;
    }

  } else {
    return DECLINED;
  }
}

/* Dispatch list of content handlers */
static const handler_rec vhome_handlers[] = { 
    { "vhome-handler", vhome_handler }, 
    { NULL }
};

/* Command Table: Allrite, all of them just work from within access.conf */

static const command_rec vhome_cmds[] = 
{
  { "VHomeTopURL", ap_set_string_slot,
    (void *) XtOffsetOf(vhome_config_rec, vhome_TopURL),
    ACCESS_CONF, TAKE1, 
    "VHome Top URL (slashed, as in '/vhome/')" },
  { "VHomePromoURL", ap_set_string_slot,
    (void *) XtOffsetOf(vhome_config_rec, vhome_PromoURL),
    ACCESS_CONF, TAKE1, 
    "VHome Promotional URL" },
  { "VHomeTable", ap_set_string_slot,
	(void*) XtOffsetOf(vhome_config_rec, vhome_Table),
	ACCESS_CONF, TAKE1,
	"VHome table"}, 
  { NULL }
};

/* Dispatch list for API hooks */
module MODULE_VAR_EXPORT vhome_module = {
    STANDARD_MODULE_STUFF, 
    vhome_init,              /* module initializer                  */
    vhome_create_dir_config, /* create per-dir config structures    */
    NULL,                    /* merge  per-dir    config structures */
    NULL,                    /* create per-server config structures */
    NULL,                    /* merge  per-server config structures */
    vhome_cmds,              /* table of config file commands       */
    vhome_handlers,          /* [#8] MIME-typed-dispatched handlers */
    NULL,                    /* [#1] URI to filename translation    */
    NULL,                    /* [#4] validate user id from request  */
    NULL,                    /* [#5] check if the user is ok _here_ */
    NULL,                    /* [#3] check access by host address   */
    NULL,                    /* [#6] determine MIME type            */
    NULL,                    /* [#7] pre-run fixups                 */
    NULL,                    /* [#9] log a transaction              */
    NULL,                    /* [#2] header parser                  */
    NULL,                    /* child_init                          */
    NULL,                    /* child_exit                          */
    NULL                     /* [#0] post read-request              */
};

