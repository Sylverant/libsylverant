/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011, 2016, 2018 Lawrence Sebald

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
#include <errno.h>

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

/* The list of language codes */
#define LANGUAGE_CODE_COUNT     8

static const char language_codes[LANGUAGE_CODE_COUNT][3] = {
    "jp", "en", "de", "fr", "es", "cs", "ct", "kr"
};

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
    cur->dbcfg.type = (char *)type;
    cur->dbcfg.host = (char *)host;
    cur->dbcfg.user = (char *)user;
    cur->dbcfg.pass = (char *)pass;
    cur->dbcfg.db = (char *)db;

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for database: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cur->dbcfg.port = (uint16_t)rv2;
    rv = 0;

err:
    xmlFree(port);
    return rv;
}

static int handle_server(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *ip, *ip6;
    int rv;

    /* Grab the attributes of the tag. */
    ip = xmlGetProp(n, XC"addr");
    ip6 = xmlGetProp(n, XC"ip6");

    /* Make sure we have what we need... */
    if(!ip) {
        debug(DBG_ERROR, "IP not given for server\n");
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

    /* See if we have a configured IPv6 address */
    if(ip6) {
        rv = inet_pton(AF_INET6, (char *)ip6, cur->server_ip6);

        /* This isn't actually fatal, for now, anyway. */
        if(rv < 1) {
            debug(DBG_WARN, "Invalid IPv6 address given: %s\n", (char *)ip6);
        }
    }

    rv = 0;

err:
    xmlFree(ip6);
    xmlFree(ip);
    return rv;
}

static int handle_shipgate(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *port, *cert, *key, *ca;
    int rv = 0;
    unsigned long rv2;

    /* Grab the attributes of the tag. */
    port = xmlGetProp(n, XC"port");
    cert = xmlGetProp(n, XC"cert");
    key = xmlGetProp(n, XC"key");
    ca = xmlGetProp(n, XC"ca-cert");

    /* Make sure we have what we need... */
    if(!port || !cert || !key || !ca) {
        debug(DBG_ERROR, "One or more required parameters not given for "
              "shipgate\n");
        rv = -1;
        goto err;
    }

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for shipgate: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cur->shipgate_port = (uint16_t)rv2;

    /* Grab the certificate file */
    cur->shipgate_cert = (char *)cert;
    cur->shipgate_key = (char *)key;
    cur->shipgate_ca = (char *)ca;

err:
    if(rv < 0) {
        xmlFree(ca);
        xmlFree(key);
        xmlFree(cert);
    }

    xmlFree(port);
    return rv;
}

static int handle_quests(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->quests_dir = (char *)fn;
        return 0;
    }

    /* If we don't have either, report the error */
    debug(DBG_ERROR, "Malformed quest tag, no dir given\n");
    return -1;
}

static int handle_limits(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *fn, *name, *enforce, *id;
    int enf = 0, i;
    void *tmp;
    unsigned long idn = 0xFFFFFFFF;

    /* Grab the attributes of the tag. */
    id = xmlGetProp(n, XC"id");
    fn = xmlGetProp(n, XC"file");
    name = xmlGetProp(n, XC"name");
    enforce = xmlGetProp(n, XC"enforce");

    /* Make sure we have the data */
    if(!fn) {
        debug(DBG_ERROR, "Limits file not given in limits tag.\n");
        goto err;
    }

    if((!id && name) || (id && !name)) {
        debug(DBG_ERROR, "Must give both or none of id and name for limits.\n");
        goto err;
    }

    if(enforce) {
        if(!xmlStrcmp(enforce, XC"true")) {
            enf = 1;
        }
        else if(xmlStrcmp(enforce, XC"false")) {
            debug(DBG_ERROR, "Invalid enforce value for limits file: %s\n",
                  (char *)enforce);
            goto err;
        }
    }
    /* This is really !id && !name, but per the above, if one is not set, then
       the other must not be set either, so this works. */
    else if(!id) {
        enf = 1;
    }

    /* Make sure we don't already have an enforced limits file. */
    if(enf && cur->limits_enforced != -1) {
        debug(DBG_ERROR, "Cannot have more than one enforced limits file!\n");
        goto err;
    }

    if(id) {
        /* Parse the id out */
        idn = (uint32_t)strtoul((char *)id, NULL, 0);

        if(idn < 0x80000000 || idn > 0xFFFFFFFF) {
            debug(DBG_ERROR, "Invalid id given for limits: %s\n", (char *)id);
            goto err;
        }

        /* Check for duplicate IDs. */
        for(i = 0; i < cur->limits_count; ++i) {
            if(cur->limits[i].id == idn) {
                debug(DBG_ERROR, "Duplicate id given for limits: %s\n",
                      (char *)id);
                goto err;
            }
        }
    }

    /* Allocate space for it in the array. */
    if(!(tmp = realloc(cur->limits, (cur->limits_count + 1) *
                                    sizeof(sylverant_limit_config_t)))) {
        debug(DBG_ERROR, "Cannot allocate space for limits file: %s\n",
              strerror(errno));
        goto err;
    }

    /* Copy it over to the struct */
    cur->limits = (sylverant_limit_config_t *)tmp;
    cur->limits[cur->limits_count].id = idn;
    cur->limits[cur->limits_count].name = (char *)name;
    cur->limits[cur->limits_count].filename = (char *)fn;
    cur->limits[cur->limits_count].enforce = enf;

    if(enf)
        cur->limits_enforced = cur->limits_count;

    ++cur->limits_count;

    xmlFree(enforce);
    xmlFree(id);

    return 0;

err:
    xmlFree(id);
    xmlFree(fn);
    xmlFree(name);
    xmlFree(enforce);
    return -1;
}

static int handle_info(xmlNode *n, sylverant_config_t *cur, int is_motd) {
    xmlChar *fn, *desc, *gc, *ep3, *bb, *lang;
    void *tmp;
    int rv = 0, count = cur->info_file_count, i, done = 0;
    char *lasts, *token;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");
    desc = xmlGetProp(n, XC"desc");
    gc = xmlGetProp(n, XC"gc");
    ep3 = xmlGetProp(n, XC"ep3");
    bb = xmlGetProp(n, XC"bb");
    lang = xmlGetProp(n, XC"languages");

    /* Make sure we have all of them... */
    if(!fn || !gc || !ep3 || !bb) {
        debug(DBG_ERROR, "Incomplete info tag\n");
        rv = -1;
        goto err;
    }

    if(!desc && !is_motd) {
        debug(DBG_ERROR, "Incomplete info tag\n");
        rv = -1;
        goto err;
    }
    else if(desc && is_motd) {
        debug(DBG_ERROR, "MOTD should not have description!\n");
        rv = -3;
        goto err;
    }

    /* Allocate space for the new description. */
    tmp = realloc(cur->info_files, (count + 1) * sizeof(sylverant_info_file_t));
    if(!tmp) {
        debug(DBG_ERROR, "Couldn't allocate space for info file\n");
        perror("realloc");
        rv = -2;
        goto err;
    }

    cur->info_files = (sylverant_info_file_t *)tmp;

    /* Copy the data in */
    cur->info_files[count].versions = 0;
    cur->info_files[count].filename = (char *)fn;
    cur->info_files[count].desc = (char *)desc;

    /* Fill in the applicable versions */
    if(!xmlStrcmp(gc, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_GC;
    }

    if(!xmlStrcmp(ep3, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_EP3;
    }

    if(!xmlStrcmp(bb, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_BB;
    }

    /* Parse the languages string, if given. */
    if(lang) {
        token = strtok_r((char *)lang, ", ", &lasts);

        while(token) {
            for(i = 0; i < LANGUAGE_CODE_COUNT && !done; ++i) {
                if(!strcmp(token, language_codes[i])) {
                    cur->info_files[count].languages |= (1 << i);
                    done = 1;
                }
            }

            if(!done) {
                debug(DBG_WARN, "Ignoring unknown language in info/motd tag on "
                      "line %hu: %s\n", n->line, token);
            }

            done = 0;
            token = strtok_r(NULL, ", ", &lasts);
        }
    }
    else {
        cur->info_files[count].languages = 0xFFFFFFFF;
    }

    ++cur->info_file_count;

    xmlFree(lang);
    xmlFree(bb);
    xmlFree(ep3);
    xmlFree(gc);

    return 0;

err:
    xmlFree(lang);
    xmlFree(bb);
    xmlFree(ep3);
    xmlFree(gc);
    xmlFree(fn);
    xmlFree(desc);
    return rv;
}

static int handle_log(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *dir, *pfx;

    /* Grab the data from the tag */
    dir = xmlGetProp(n, XC"dir");
    pfx = xmlGetProp(n, XC"prefix");

    /* If we don't have the directory, report an error */
    if(!dir) {
        debug(DBG_ERROR, "Malformed log tag, no dir given\n");
        goto err;
    }

    /* If no prefix was given, then blank it out. */
    if(!pfx) {
        pfx = (xmlChar *)xmlMalloc(1);
        pfx[0] = 0;
    }

    /* Save the data to the configuration struct. */
    cur->log_dir = (char *)dir;
    cur->log_prefix = (char *)pfx;
    return 0;

err:
    xmlFree(dir);
    xmlFree(pfx);
    return -1;
}

static int handle_patch(xmlNode *n, sylverant_config_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->patch_dir = (char *)fn;
        return 0;
    }

    /* If we don't have either, report the error */
    debug(DBG_ERROR, "Malformed patch tag, no dir given\n");
    return -1;
}

int sylverant_read_config(const char *f, sylverant_config_t **cfg) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    int irv = 0;
    sylverant_config_t *rv;

    /* Allocate space for the base of the config. */
    rv = (sylverant_config_t *)malloc(sizeof(sylverant_config_t));

    if(!rv) {
        *cfg = NULL;
        debug(DBG_ERROR, "Couldn't allocate space for config\n");
        perror("malloc");
        return -1;
    }

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_config_t));
    rv->limits_enforced = -1;

    /* Create an XML Parsing context */
    cxt = xmlNewParserCtxt();
    if(!cxt) {
        debug(DBG_ERROR, "Couldn't create parsing context for config\n");
        irv = -2;
        goto err;
    }

    /* Open the configuration file for reading. */
    if(f) {
        doc = xmlReadFile(f, NULL, XML_PARSE_DTDVALID);
    }
    else {
        doc = xmlReadFile(sylverant_cfg, NULL, XML_PARSE_DTDVALID);
    }

    if(!doc) {
        xmlParserError(cxt, "Error in parsing config");
        irv = -3;
        goto err_cxt;
    }

    /* Make sure the document validated properly. */
    if(!cxt->valid) {
        xmlParserValidityError(cxt, "Validity Error parsing config");
        irv = -4;
        goto err_doc;
    }

    /* If we've gotten this far, we have a valid document, now go through and
       add in entries for everything... */
    n = xmlDocGetRootElement(doc);

    if(!n) {
        debug(DBG_WARN, "Empty config document\n");
        irv = -5;
        goto err_doc;
    }

    /* Make sure the config looks sane. */
    if(xmlStrcmp(n->name, XC"sylverant_config")) {
        debug(DBG_WARN, "Config does not appear to be the right type\n");
        irv = -6;
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
            if(handle_database(n, rv)) {
                irv = -7;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"server")) {
            if(handle_server(n, rv)) {
                irv = -8;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"quests")) {
            if(handle_quests(n, rv)) {
                irv = -9;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"limits")) {
            if(handle_limits(n, rv)) {
                irv = -10;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"info")) {
            if(handle_info(n, rv, 0)) {
                irv = -11;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"motd")) {
            if(handle_info(n, rv, 1)) {
                irv = -12;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"shipgate")) {
            if(handle_shipgate(n, rv)) {
                irv = -13;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"log")) {
            if(handle_log(n, rv)) {
                irv = -14;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"patch")) {
            if(handle_patch(n, rv)) {
                irv = -15;
                goto err_doc;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

    *cfg = rv;

    /* Cleanup/error handling below... */
err_doc:
    xmlFreeDoc(doc);
err_cxt:
    xmlFreeParserCtxt(cxt);
err:
    if(irv && irv > -7) {
        free(rv);
        *cfg = NULL;
    }
    else if(irv) {
        sylverant_free_config(rv);
        *cfg = NULL;
    }

    return irv;
}

void sylverant_free_config(sylverant_config_t *cfg) {
    int i;

    /* Make sure we actually have a valid configuration pointer. */
    if(cfg) {
        for(i = 0; i < cfg->info_file_count; ++i) {
            xmlFree(cfg->info_files[i].filename);
            xmlFree(cfg->info_files[i].desc);
        }

        for(i = 0; i < cfg->limits_count; ++i) {
            xmlFree(cfg->limits[i].filename);
            xmlFree(cfg->limits[i].name);
        }

        /* Clean up the pointers */
        xmlFree(cfg->dbcfg.type);
        xmlFree(cfg->dbcfg.host);
        xmlFree(cfg->dbcfg.user);
        xmlFree(cfg->dbcfg.pass);
        xmlFree(cfg->dbcfg.db);
        xmlFree(cfg->shipgate_cert);
        xmlFree(cfg->shipgate_key);
        xmlFree(cfg->shipgate_ca);
        xmlFree(cfg->quests_dir);
        xmlFree(cfg->log_dir);
        xmlFree(cfg->log_prefix);
        xmlFree(cfg->patch_dir);

        free(cfg->info_files);
        free(cfg->limits);

        /* Clean up the base structure. */
        free(cfg);
    }
}
