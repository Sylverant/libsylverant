/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011, 2012, 2013 Lawrence Sebald

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
#include <ctype.h>
#include <limits.h>
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

static int handle_shipgate(xmlNode *n, sylverant_ship_t *cfg) {
    xmlChar *ip, *port, *ca, *addr;
    int rv;
    unsigned long rv2;
    uint32_t ip4;
    uint8_t ip6[16];

    /* Grab the attributes of the tag. */
    ip = xmlGetProp(n, XC"ip");
    port = xmlGetProp(n, XC"port");
    ca = xmlGetProp(n, XC"ca");
    addr = xmlGetProp(n, XC"addr");

    /* Make sure we have all of them... */
    if(!port || !ca) {
        debug(DBG_ERROR, "port or CA not given for shipgate\n");
        rv = -1;
        goto err;
    }
    else if((addr && ip) || (!addr && !ip)) {
        debug(DBG_ERROR, "You must provide ONE of ip or addr for shipgate\n");
        rv = -4;
        goto err;
    }

    /* Parse the address address out */
    if(addr) {
        cfg->shipgate_host = addr;
    }
    else if(ip) {
        rv = inet_pton(AF_INET, (char *)ip, &ip4);

        if(rv < 1) {
            rv = inet_pton(AF_INET6, (char *)ip, ip6);

            if(rv < 1) {
                debug(DBG_ERROR, "Invalid IP address given for shipgate: %s\n",
                      (char *)ip);
                rv = -2;
                goto err;
            }
        }

        cfg->shipgate_host = ip;
    }

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for shipgate: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cfg->shipgate_port = (uint16_t)rv2;
    cfg->shipgate_ca = ca;
    rv = 0;

err:
    xmlFree(port);

    if(rv) {
        xmlFree(ip);
        xmlFree(ca);
        xmlFree(addr);
    }

    return rv;
}

static int handle_net(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *ip, *port, *ip6, *addr, *addr6;
    int rv;
    unsigned long rv2;
    uint32_t tmpip4;
    uint8_t tmpip6[16];

    /* Grab the attributes of the tag. */
    ip = xmlGetProp(n, XC"ip");
    port = xmlGetProp(n, XC"port");
    ip6 = xmlGetProp(n, XC"ip6");
    addr = xmlGetProp(n, XC"addr");
    addr6 = xmlGetProp(n, XC"addr6");

    /* Make sure we have both of the required ones... */
    if(!port) {
        debug(DBG_ERROR, "Port not given for ship\n");
        rv = -1;
        goto err;
    }
    else if((ip && addr) || (!ip && !addr)) {
        debug(DBG_ERROR, "Must give ONE of IPv4 or addr for ship\n");
        rv = -4;
        goto err;
    }
    else if(ip6 && addr6) {
        debug(DBG_ERROR, "Cannot give both ip6 and addr6 for ship\n");
        rv = -5;
        goto err;
    }

    /* Parse the IP address out */
    if(ip) {
        rv = inet_pton(AF_INET, (char *)ip, &tmpip4);

        if(rv < 1) {
            debug(DBG_ERROR, "Invalid IPv4 address given for ship: %s\n",
                  (char *)ip);
            rv = -2;
            goto err;
        }

        cur->ship_host = (char *)ip;
    }
    else {
        cur->ship_host = (char *)addr;
    }

    /* Parse the port out */
    rv2 = strtoul((char *)port, NULL, 0);

    if(rv2 == 0 || rv2 > 0xFFFF) {
        debug(DBG_ERROR, "Invalid port given for ship: %s\n", (char *)port);
        rv = -3;
        goto err;
    }

    cur->base_port = (uint16_t)rv2;

    /* See if we have a configured IPv6 address */
    if(ip6) {
        rv = inet_pton(AF_INET6, (char *)ip6, tmpip6);

        if(rv < 1) {
            debug(DBG_WARN, "Invalid IPv6 address given for ship: %s\n",
                  (char *)ip6);
            rv = -6;
            goto err;
        }

        cur->ship_host6 = (char *)ip6;
    }
    else {
        cur->ship_host6 = (char *)addr6;
    }

    rv = 0;

err:
    xmlFree(port);

    if(rv) {
        xmlFree(ip);
        xmlFree(ip6);
        xmlFree(addr);
        xmlFree(addr6);
    }

    return rv;
}

static int handle_event_old(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *game, *lobby;
    int rv;
    long rv2;

    /* Make sure we don't already have an event setup done... */
    if(cur->event_count != 0) {
        debug(DBG_WARN, "Ignoring <event> tag on line %hu\n", n->line);
        return 0;
    }

    /* Grab the attributes of the tag. */
    game = xmlGetProp(n, XC"game");
    lobby = xmlGetProp(n, XC"lobby");

    /* Make sure we have the data */
    if(!game || !lobby) {
        debug(DBG_ERROR, "Event number(s) not given\n");
        rv = -1;
        goto err;
    }

    /* Parse the game event out */
    errno = 0;
    rv2 = strtol((char *)game, NULL, 0);

    if(errno || rv2 > 6 || rv2 < 0) {
        debug(DBG_ERROR, "Invalid game event given for ship: %s\n",
              (char *)game);
        rv = -2;
        goto err;
    }

    cur->events[0].game_event = (uint8_t)rv2;

    /* Parse the lobby event out */
    rv2 = strtol((char *)lobby, NULL, 0);

    if(errno || rv2 > 14 || rv2 < 0) {
        debug(DBG_ERROR, "Invalid lobby event given for ship: %s\n",
              (char *)lobby);
        rv = -3;
        goto err;
    }

    cur->events[0].lobby_event = (uint8_t)rv2;
    rv = 0;
    cur->event_count = 1;

err:
    xmlFree(game);
    xmlFree(lobby);
    return rv;
}

static int handle_event_date(xmlNode *n, uint8_t *m, uint8_t *d) {
    xmlChar *month, *day;
    int rv;

    /* Grab the attributes of the tag. */
    month = xmlGetProp(n, XC"month");
    day = xmlGetProp(n, XC"day");

    /* Make sure we have the data */
    if(!month || !day) {
        debug(DBG_ERROR, "Event timeframe not specified properly\n");
        rv = -1;
        goto err;
    }

    /* Parse the game event out */
    errno = 0;
    *m = (uint8_t)strtoul((char *)month, NULL, 0);

    if(errno || *m > 12 || *m < 1) {
        debug(DBG_ERROR, "Invalid month given: %s\n", (char *)month);
        rv = -3;
        goto err;
    }

    /* Parse the lobby event out */
    *d = (uint8_t)strtoul((char *)day, NULL, 0);

    if(errno || *d < 1) {
        debug(DBG_ERROR, "Invalid day given: %s\n", (char *)day);
        rv = -4;
        goto err;
    }

    /* Check the day for validity */
    if(*m == 4 || *m == 6 || *m == 9 || *m == 11) {
       if(*d > 30) {
            debug(DBG_ERROR, "Invalid day given (month = %d): %d\n", (int)*m,
                  (int)*d);
            rv = -5;
            goto err;
       }
    }
    else if(*m == 2) {
        if(*d > 29) {
            debug(DBG_ERROR, "Invalid day given (month = %d): %d\n", (int)*m,
                  (int)*d);
            rv = -6;
            goto err;
        }
    }
    else {
        if(*d > 31) {
            debug(DBG_ERROR, "Invalid day given (month = %d): %d\n", (int)*m,
                  (int)*d);
            rv = -7;
            goto err;
        }
    }

    rv = 0;

err:
    xmlFree(day);
    xmlFree(month);
    return rv;
}

static int handle_event(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *game, *lobby;
    int rv;
    long rv2;
    int c = cur->event_count;
    void *tmp;
    xmlNode *n2;

    /* Make sure we already have an event setup started... */
    if(c < 1) {
        debug(DBG_WARN, "Ignoring <event> tag on line %hu\n", n->line);
        return 0;
    }

    /* Grab the attributes of the tag. */
    game = xmlGetProp(n, XC"game");
    lobby = xmlGetProp(n, XC"lobby");

    /* Make sure we have the data */
    if(!game || !lobby) {
        debug(DBG_ERROR, "Event number(s) not given\n");
        rv = -1;
        goto err;
    }

    /* Allocate the space we need */
    tmp = realloc(cur->events, sizeof(sylverant_event_t) * (c + 1));
    if(!tmp) {
        debug(DBG_ERROR, "Cannot allocate memory for event: %s\n",
              strerror(errno));
        rv = -2;
        goto err;
    }

    cur->events = (sylverant_event_t *)tmp;
    memset(&cur->events[c], 0, sizeof(sylverant_event_t));

    /* Parse the game event out */
    errno = 0;
    rv2 = strtol((char *)game, NULL, 0);

    if(errno || rv2 > 6 || rv2 < -1) {
        debug(DBG_ERROR, "Invalid game event given for ship: %s\n",
              (char *)game);
        rv = -3;
        goto err;
    }

    cur->events[c].game_event = (uint8_t)rv2;

    /* Parse the lobby event out */
    rv2 = strtol((char *)lobby, NULL, 0);

    if(errno || rv2 > 14 || rv2 < -1) {
        debug(DBG_ERROR, "Invalid lobby event given for ship: %s\n",
              (char *)lobby);
        rv = -4;
        goto err;
    }

    cur->events[c].lobby_event = (uint8_t)rv2;

    /* Parse out the children of the <event> tag. */
    n2 = n->children;
    while(n2) {
        if(n2->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n2 = n2->next;
            continue;
        }
        else if(!xmlStrcmp(n2->name, XC"start")) {
            if(handle_event_date(n2, &cur->events[c].start_month,
                                 &cur->events[c].start_day)) {
                rv = -5;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"end")) {
            if(handle_event_date(n2, &cur->events[c].end_month,
                                 &cur->events[c].end_day)) {
                rv = -6;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n2->name,
                  n2->line);
        }

        n2 = n2->next;
    }

    ++cur->event_count;
    rv = 0;

err:
    xmlFree(game);
    xmlFree(lobby);
    return rv;
}

static int handle_info(xmlNode *n, sylverant_ship_t *cur, int is_motd) {
    xmlChar *fn, *desc, *v1, *v2, *pc, *lang;
    void *tmp;
    int rv = 0, count = cur->info_file_count, i, done = 0;
    char *lasts, *token;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");
    desc = xmlGetProp(n, XC"desc");
    v1 = xmlGetProp(n, XC"v1");
    v2 = xmlGetProp(n, XC"v2");
    pc = xmlGetProp(n, XC"pc");
    lang = xmlGetProp(n, XC"languages");

    /* Make sure we have all of them... */
    if(!fn || (!desc && !is_motd)) {
        debug(DBG_ERROR, "Incomplete info/motd tag\n");
        rv = -1;
        goto err;
    }

    if(is_motd && desc) {
        debug(DBG_ERROR, "MOTD should not have description!\n");
        rv = -3;
        goto err;
    }

    /* Allocate space for the new description. */
    tmp = realloc(cur->info_files, (count + 1) * sizeof(sylverant_info_file_t));
    if(!tmp) {
        debug(DBG_ERROR, "Couldn't allocate space for info/motd file\n");
        perror("realloc");
        rv = -2;
        goto err;
    }

    cur->info_files = (sylverant_info_file_t *)tmp;

    /* Copy the data in */
    cur->info_files[count].versions = 0;
    cur->info_files[count].filename = fn;
    cur->info_files[count].desc = desc;

    /* Fill in the applicable versions */
    if(!v1 || !xmlStrcmp(v1, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_V1;
    }

    if(!v2 || !xmlStrcmp(v2, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_V2;
    }

    if(!pc || !xmlStrcmp(pc, XC"true")) {
        cur->info_files[count].versions |= SYLVERANT_INFO_PC;
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
    xmlFree(pc);
    xmlFree(v2);
    xmlFree(v1);

    return 0;

err:
    xmlFree(lang);
    xmlFree(pc);
    xmlFree(v2);
    xmlFree(v1);
    xmlFree(fn);
    xmlFree(desc);
    return rv;
}

static int handle_quests(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->quests_dir = (char *)fn;
        return 0;
    }

    /* If not, see if we have the file attribute */
    if((fn = xmlGetProp(n, XC"file"))) {
        cur->quests_file = (char *)fn;
        return 0;
    }

    /* If we don't have either, report the error */
    debug(DBG_ERROR, "Malformed quest tag, no file or dir given\n");
    return -1;
}

static int handle_limits(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");

    /* Make sure we have the data */
    if(!fn) {
        debug(DBG_ERROR, "Limits filename not given\n");
        return -1;
    }

    /* Copy it over to the struct */
    cur->limits_file = (char *)fn;

    return 0;
}

static int handle_bans(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");

    /* Make sure we have the data */
    if(!fn) {
        debug(DBG_ERROR, "Bans filename not given\n");
        return -1;
    }

    /* Copy it over to the struct */
    cur->bans_file = (char *)fn;

    return 0;
}

static int handle_scripts(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the attributes of the tag. */
    fn = xmlGetProp(n, XC"file");

    /* Make sure we have the data */
    if(!fn) {
        debug(DBG_ERROR, "Scripts filename not given\n");
        return -1;
    }

    /* Copy it over to the struct */
    cur->scripts_file = (char *)fn;

    return 0;
}

static int handle_versions(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *v1, *v2, *pc, *gc, *ep3, *bb;
    int rv = 0;

    /* Grab the attributes of the tag. */
    v1 = xmlGetProp(n, XC"v1");
    v2 = xmlGetProp(n, XC"v2");
    pc = xmlGetProp(n, XC"pc");
    gc = xmlGetProp(n, XC"gc");
    ep3 = xmlGetProp(n, XC"ep3");
    bb = xmlGetProp(n, XC"bb");

    /* Make sure we have the data */
    if(!v1 || !v2 || !pc || !gc || !ep3) {
        debug(DBG_ERROR, "Missing version\n");
        rv = -1;
        goto err;
    }

    /* Parse everything out */
    if(!xmlStrcmp(v1, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOV1;
    }

    if(!xmlStrcmp(v2, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOV2;
    }

    if(!xmlStrcmp(pc, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOPC;
    }

    if(!xmlStrcmp(gc, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOEP12;
    }

    if(!xmlStrcmp(ep3, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOEP3;
    }

    if(bb && !xmlStrcmp(bb, XC"false")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_NOBB;
    }

err:    
    xmlFree(v1);
    xmlFree(v2);
    xmlFree(pc);
    xmlFree(gc);
    xmlFree(ep3);
    xmlFree(bb);
    return rv;
}

static int handle_events(xmlNode *n, sylverant_ship_t *cur) {
    int rv;
    unsigned long rv2;
    xmlNode *n2;

    /* Make sure we don't already have an event setup done... */
    if(cur->event_count != 0) {
        debug(DBG_WARN, "Ignoring <events> tag on line %hu\n", n->line);
        return 0;
    }

    /* Parse out the children of the <events> tag. */
    n2 = n->children;
    while(n2) {
        if(n2->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n2 = n2->next;
            continue;
        }
        else if(!xmlStrcmp(n2->name, XC"defaults")) {
            /* Cheat a bit here, and reuse the old <event> tag parsing code.
               Since this has to be first, that should work fine... */
            if(handle_event_old(n2, cur)) {
                rv = -1;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"event")) {
            if(handle_event(n2, cur)) {
                rv = -2;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n2->name,
                  n2->line);
        }

        n2 = n2->next;
    }

    rv = 0;

err:
    return rv;
}

static int handle_bbparam(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->bb_param_dir = (char *)fn;
        return 0;
    }

    /* If we don't have it, report the error */
    debug(DBG_ERROR, "Malformed bbparam tag, no dir given\n");
    return -1;
}

static int handle_bbmaps(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->bb_map_dir = (char *)fn;
        return 0;
    }

    /* If we don't have it, report the error */
    debug(DBG_ERROR, "Malformed bbmaps tag, no dir given\n");
    return -1;
}

static int handle_itempt(xmlNode *n, sylverant_ship_t *cur) {
    /* Grab the ptdata filenames */
    cur->v2_ptdata_file = (char *)xmlGetProp(n, XC"v2");
    cur->gc_ptdata_file = (char *)xmlGetProp(n, XC"gc");
    cur->bb_ptdata_file = (char *)xmlGetProp(n, XC"bb");

    return 0;
}

static int handle_v2maps(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *fn;

    /* Grab the directory, if given */
    if((fn = xmlGetProp(n, XC"dir"))) {
        cur->v2_map_dir = (char *)fn;
        return 0;
    }

    /* If we don't have it, report the error */
    debug(DBG_ERROR, "Malformed v2maps tag, no dir given\n");
    return -1;
}

static int handle_itempmt(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *limit;

    /* Grab the pmtdata filenames */
    cur->v2_pmtdata_file = (char *)xmlGetProp(n, XC"v2");
    cur->gc_pmtdata_file = (char *)xmlGetProp(n, XC"gc");
    cur->bb_pmtdata_file = (char *)xmlGetProp(n, XC"bb");

    /* See if we're supposed to cap unit +/- values like the client does... */
    limit = xmlGetProp(n, XC"limitv2units");
    if(!limit || !xmlStrcmp(limit, "true"))
        cur->local_flags |= SYLVERANT_SHIP_PMT_LIMITV2;

    xmlFree(limit);

    limit = xmlGetProp(n, XC"limitgcunits");
    if(!limit || !xmlStrcmp(limit, "true"))
        cur->local_flags |= SYLVERANT_SHIP_PMT_LIMITGC;

    xmlFree(limit);

    limit = xmlGetProp(n, XC"limitbbunits");
    if(!limit || !xmlStrcmp(limit, "true"))
        cur->local_flags |= SYLVERANT_SHIP_PMT_LIMITBB;

    xmlFree(limit);

    return 0;
}

static int handle_itemrt(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *quest;

    /* Grab the rtdata filenames */
    cur->v2_rtdata_file = (char *)xmlGetProp(n, XC"v2");
    cur->gc_rtdata_file = (char *)xmlGetProp(n, XC"gc");
    cur->bb_rtdata_file = (char *)xmlGetProp(n, XC"bb");

    quest = xmlGetProp(n, XC"questrares");

    /* See if we're supposed to disable quest rares globally. */
    if(quest) {
        if(!xmlStrcmp(quest, "true"))
            cur->local_flags |= SYLVERANT_SHIP_QUEST_RARES |
                SYLVERANT_SHIP_QUEST_SRARES;
        else if(!xmlStrcmp(quest, "partial"))
            cur->local_flags |= SYLVERANT_SHIP_QUEST_SRARES;
    }

    xmlFree(quest);

    return 0;
}

static int handle_ship(xmlNode *n, sylverant_ship_t *cur) {
    xmlChar *name, *blocks, *key, *gms, *menu, *gmonly, *cert;
    int rv;
    unsigned long rv2;
    xmlNode *n2;

    /* Grab the attributes of the <ship> tag. */
    name = xmlGetProp(n, XC"name");
    blocks = xmlGetProp(n, XC"blocks");
    key = xmlGetProp(n, XC"key");
    gms = xmlGetProp(n, XC"gms");
    menu = xmlGetProp(n, XC"menu");
    gmonly = xmlGetProp(n, XC"gmonly");
    cert = xmlGetProp(n, XC"cert");

    if(!name || !blocks || !key || !gms || !gmonly || !menu || !cert) {
        debug(DBG_ERROR, "Required attribute of ship not found\n");
        rv = -1;
        goto err;
    }

    /* Copy out the strings out that we need */
    cur->name = (char *)name;
    cur->ship_key = (char *)key;
    cur->ship_cert = (char *)cert;
    cur->gm_file = (char *)gms;

    /* Copy out the gmonly flag */
    if(!xmlStrcmp(gmonly, XC"true")) {
        cur->shipgate_flags |= SHIPGATE_FLAG_GMONLY;
    }

    /* Grab the menu code */
    rv = xmlStrlen(menu);

    if(rv == 2 && isalpha(menu[0]) && isalpha(menu[1])) {
        cur->menu_code = ((uint16_t)menu[0]) | (((uint16_t)(menu[1])) << 8);
    }
    else if(rv) {
        debug(DBG_ERROR, "Invalid menu code given");
        rv = -2;
        goto err;
    }

    /* Copy out the number of blocks */
    rv2 = strtoul((char *)blocks, NULL, 0);

    if(rv2 == 0 || rv2 > 16) {
        debug(DBG_ERROR, "Invalid block count given: %s\n", (char *)blocks);
        rv = -3;
        goto err;
    }
    
    cur->blocks = (int)rv2;

    /* Parse out the children of the <ship> tag. */
    n2 = n->children;
    while(n2) {
        if(n2->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n2 = n2->next;
            continue;
        }
        else if(!xmlStrcmp(n2->name, XC"net")) {
            if(handle_net(n2, cur)) {
                rv = -3;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"event")) {
            if(handle_event_old(n2, cur)) {
                rv = -4;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"info")) {
            if(handle_info(n2, cur, 0)) {
                rv = -5;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"quests")) {
            if(handle_quests(n2, cur)) {
                rv = -6;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"limits")) {
            if(handle_limits(n2, cur)) {
                rv = -7;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"motd")) {
            if(handle_info(n2, cur, 1)) {
                rv = -8;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"versions")) {
            if(handle_versions(n2, cur)) {
                rv = -9;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"bans")) {
            if(handle_bans(n2, cur)) {
                rv = -10;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"scripts")) {
            if(handle_scripts(n2, cur)) {
                rv = -11;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"events")) {
            if(handle_events(n2, cur)) {
                rv = -12;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"bbparam")) {
            if(handle_bbparam(n2, cur)) {
                rv = -13;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"bbmaps")) {
            if(handle_bbmaps(n2, cur)) {
                rv = -14;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"itempt")) {
            if(handle_itempt(n2, cur)) {
                rv = -15;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"v2maps")) {
            if(handle_v2maps(n2, cur)) {
                rv = -16;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"itempmt")) {
            if(handle_itempmt(n2, cur)) {
                rv = -17;
                goto err;
            }
        }
        else if(!xmlStrcmp(n2->name, XC"itemrt")) {
            if(handle_itemrt(n2, cur)) {
                rv = -18;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n2->name,
                  n2->line);
        }

        n2 = n2->next;
    }

    rv = 0;

err:
    xmlFree(blocks);
    xmlFree(gmonly);
    xmlFree(menu);
    return rv;
}

int sylverant_read_ship_config(const char *f, sylverant_ship_t **cfg) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    sylverant_ship_t *rv;
    int irv = 0;

    /* Allocate space for the base of the config. */
    rv = (sylverant_ship_t *)malloc(sizeof(sylverant_ship_t));

    if(!rv) {
        *cfg = NULL;
        debug(DBG_ERROR, "Couldn't allocate space for ship config\n");
        perror("malloc");
        return -1;
    }

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_ship_t));

    /* Allocate space for the default event. */
    rv->events = (sylverant_event_t *)malloc(sizeof(sylverant_event_t));
    if(!rv->events) {
        *cfg = NULL;
        free(rv);
        debug(DBG_ERROR, "Couldn't allocate space for event: %s\n",
              strerror(errno));
        return -1;
    }

    memset(rv->events, 0, sizeof(sylverant_event_t));

    /* Create an XML Parsing context */
    cxt = xmlNewParserCtxt();
    if(!cxt) {
        debug(DBG_ERROR, "Couldn't create parsing context for ship config\n");
        irv = -2;
        goto err;
    }

    /* Open the configuration file for reading. */
    if(f) {
        doc = xmlReadFile(f, NULL, XML_PARSE_DTDVALID);
    }
    else {
        doc = xmlReadFile(sylverant_ship_cfg, NULL, XML_PARSE_DTDVALID);
    }

    if(!doc) {
        xmlParserError(cxt, "Error in parsing ship config");
        irv = -3;
        goto err_cxt;
    }

    /* Make sure the document validated properly. */
    if(!cxt->valid) {
        xmlParserValidityError(cxt, "Validity Error parsing ship config");
        irv = -4;
        goto err_doc;
    }

    /* If we've gotten this far, we have a valid document, now go through and
       add in entries for everything... */
    n = xmlDocGetRootElement(doc);

    if(!n) {
        debug(DBG_WARN, "Empty ship config document\n");
        irv = -5;
        goto err_doc;
    }

    /* Make sure the config looks sane. */
    if(xmlStrcmp(n->name, XC"ships")) {
        debug(DBG_WARN, "Ship config does not appear to be the right type\n");
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
        else if(!xmlStrcmp(n->name, XC"shipgate")) {
            if(handle_shipgate(n, rv)) {
                irv = -7;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"ship")) {
            if(handle_ship(n, rv)) {
                irv = -8;
                goto err_doc;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

    /* Did we configure a set of events, or just the default? */
    if(rv->event_count == 0) {
        rv->event_count = 1;
    }

    *cfg = rv;

    /* Cleanup/error handling below... */
err_doc:
    xmlFreeDoc(doc);
err_cxt:
    xmlFreeParserCtxt(cxt);
    xmlCleanupParser();

err:
    if(irv && irv > -7) {
        free(rv);
        *cfg = NULL;
    }
    else if(irv) {
        sylverant_free_ship_config(rv);
        *cfg = NULL;
    }

    return irv;
}

void sylverant_free_ship_config(sylverant_ship_t *cfg) {
    int j;

    /* Make sure we actually have a valid configuration pointer. */
    if(cfg) {
        if(cfg->info_files) {
            for(j = 0; j < cfg->info_file_count; ++j) {
                xmlFree(cfg->info_files[j].desc);
                xmlFree(cfg->info_files[j].filename);
            }

            free(cfg->info_files);
        }

        xmlFree(cfg->name);
        xmlFree(cfg->gm_file);
        xmlFree(cfg->limits_file);
        xmlFree(cfg->quests_file);
        xmlFree(cfg->quests_dir);
        xmlFree(cfg->bans_file);
        xmlFree(cfg->scripts_file);
        xmlFree(cfg->shipgate_ca);
        xmlFree(cfg->ship_key);
        xmlFree(cfg->ship_cert);
        xmlFree(cfg->bb_param_dir);
        xmlFree(cfg->bb_map_dir);
        xmlFree(cfg->v2_map_dir);
        xmlFree(cfg->shipgate_host);
        xmlFree(cfg->ship_host);
        xmlFree(cfg->ship_host6);
        xmlFree(cfg->v2_ptdata_file);
        xmlFree(cfg->gc_ptdata_file);
        xmlFree(cfg->bb_ptdata_file);
        xmlFree(cfg->v2_pmtdata_file);
        xmlFree(cfg->gc_pmtdata_file);
        xmlFree(cfg->bb_pmtdata_file);
        xmlFree(cfg->v2_rtdata_file);
        xmlFree(cfg->gc_rtdata_file);
        xmlFree(cfg->bb_rtdata_file);
    
        free(cfg->events);

        /* Clean up the base structure. */
        free(cfg);
    }
}
