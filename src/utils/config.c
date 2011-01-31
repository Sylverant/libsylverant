/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011 Lawrence Sebald

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License version 3
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "sylverant/config.h"
#include "sylverant/debug.h"

#ifndef LIBXML_TREE_ENABLED
#error You must have libxml2 with tree support built-in.
#endif

#define XC (const xmlChar *)

static int handle_database(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *type, *host, *user, *pass, *db, *port;
    int rv;
    unsigned long rv2;

    /* Grab the attributes of the tag. */
    type = xmlGetProp(n, XC"type");
    host = xmlGetProp(n, XC"host");
    user = xmlGetProp(n, XC"user");
    pass = xmlGetProp(n, XC"pass");
    db = xmlGetProp(n, XC"db");
    port = xmlGetProp(n, XC"port");

    /* Make sure we have all of them... */
    if(!type || !host || !user || !pass || !db || !port) {
        debug(DBG_ERROR, "Incomplete database tag\n");
        rv = -1;
        goto err;
    }

    /* Copy out the strings */
    strncpy(cur->dbcfg.type, (char *)type, 255);
    strncpy(cur->dbcfg.host, (char *)host, 255);
    strncpy(cur->dbcfg.user, (char *)user, 255);
    strncpy(cur->dbcfg.pass, (char *)pass, 255);
    strncpy(cur->dbcfg.db, (char *)db, 255);

    cur->dbcfg.type[255] = '\0';
    cur->dbcfg.host[255] = '\0';
    cur->dbcfg.user[255] = '\0';
    cur->dbcfg.pass[255] = '\0';
    cur->dbcfg.db[255] = '\0';

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for database: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cur->dbcfg.port = (unsigned int)rv2;
    rv = 0;

err:
    xmlFree(type);
    xmlFree(host);
    xmlFree(user);
    xmlFree(pass);
    xmlFree(db);
    xmlFree(port);
    return rv;
}

static int handle_server(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *ip, *port;
    int rv;
    unsigned long rv2;

    /* Grab the attributes of the tag. */
    ip = xmlGetProp(n, XC"addr");
    port = xmlGetProp(n, XC"port");

    /* Make sure we have both of them... */
    if(!ip || !port) {
        debug(DBG_ERROR, "IP or port not given for server\n");
        rv = -1;
        goto err;
    }

    /* Parse the IP address out */
    rv = inet_pton(AF_INET, (char *)ip, &cur->server_ip);

    if(rv < 1) {
        debug(DBG_ERROR, "Invalid IP address given for server: %s\n",
              (char *)ip);
        rv = -2;
        goto err;
    }

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for server: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cur->server_port = (uint16_t)rv2;
    rv = 0;

err:
    xmlFree(ip);
    xmlFree(port);
    return rv;
}

static int handle_quests(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        strncpy(cur->quests_dir, (char *)fn, 255);
        cur->quests_dir[255] = '\0';
        xmlFree(fn);
        return 0;
    }

    /* If we don't have either, report the error */
    debug(DBG_ERROR, "Malformed quest tag, no dir given\n");
    return -1;
}

static int handle_limits(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *fn;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");

    /* Make sure we have the data */
    if(!fn) {
        debug(DBG_ERROR, "Limits filename not given\n");
        return -1;
    }

    /* Copy it over to the struct */
    strncpy(cur->limits_file, (char *)fn, 255);
    cur->limits_file[255] = '\0';

    xmlFree(fn);
    return 0;
}

int sylverant_read_config(sylverant_config_t *cfg) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    int irv = 0;

    /* Clear out the config. */
    memset(cfg, 0, sizeof(sylverant_config_t));

    /* Create an XML Parsing context */
    cxt = xmlNewParserCtxt();
    if(!cxt) {
        debug(DBG_ERROR, "Couldn't create parsing context for config\n");
        irv = -1;
        goto err;
    }

    /* Open the configuration file for reading. */
    doc = xmlReadFile(sylverant_cfg, NULL, XML_PARSE_DTDVALID);

    if(!doc) {
        xmlParserError(cxt, "Error in parsing config");
        irv = -2;
        goto err_cxt;
    }

    /* Make sure the document validated properly. */
    if(!cxt->valid) {
        xmlParserValidityError(cxt, "Validity Error parsing config");
        irv = -3;
        goto err_doc;
    }

    /* If we've gotten this far, we have a valid document, now go through and
       add in entries for everything... */
    n = xmlDocGetRootElement(doc);

    if(!n) {
        debug(DBG_WARN, "Empty config document\n");
        irv = -4;
        goto err_doc;
    }

    /* Make sure the config looks sane. */
    if(xmlStrcmp(n->name, XC"sylverant_config")) {
        debug(DBG_WARN, "Config does not appear to be the right type\n");
        irv = -5;
        goto err_doc;
    }

    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"database")) {
            if(handle_database(n, cfg)) {
                irv = -6;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"server")) {
            if(handle_server(n, cfg)) {
                irv = -7;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"quests")) {
            if(handle_quests(n, cfg)) {
                irv = -8;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"limits")) {
            if(handle_limits(n, cfg)) {
                irv = -9;
                goto err_doc;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

    /* Cleanup/error handling below... */
err_doc:
    xmlFreeDoc(doc);
err_cxt:
    xmlFreeParserCtxt(cxt);
err:
    return irv;
}
