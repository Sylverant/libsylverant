/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011, 2014, 2015, 2018, 2019, 2020 Lawrence Sebald

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
#include <unistd.h>
#include <time.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "sylverant/quest.h"
#include "sylverant/debug.h"
#include "sylverant/memory.h"

#ifndef LIBXML_TREE_ENABLED
#error You must have libxml2 with tree support built-in.
#endif

#define XC (const xmlChar *)

#ifndef HAVE_TIMEGM
#ifdef HAVE__MKGMTIME
#define timegm _mkgmtime
#else
#warning Time values will not be in UTC time unless run in UTC timezone.
#define timegm mktime
#endif
#endif

static void quest_dtor(void *o);

static int handle_long(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *desc;

    /* Grab the long description from the node */
    if((desc = xmlNodeListGetString(n->doc, n->children, 1))) {
        q->long_desc = (char *)desc;
    }

    return 0;
}

static int handle_short(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *desc;

    /* Grab the short description from the node */
    if((desc = xmlNodeListGetString(n->doc, n->children, 1))) {
        strncpy(q->desc, (const char *)desc, 111);
        q->desc[111] = '\0';
        xmlFree(desc);
    }

    return 0;
}

static int handle_monster(xmlNode *n, sylverant_quest_t *q, uint32_t def,
                          uint32_t mask) {
    xmlChar *id, *type, *drops;
    int rv = 0, count;
    void *tmp;
    uint32_t drop;
    uint32_t num;
    typedef struct sylverant_quest_enemy mon;

    /* Grab the attributes we're expecting */
    type = xmlGetProp(n, XC"type");
    id = xmlGetProp(n, XC"id");
    drops = xmlGetProp(n, XC"drops");

    /* Make sure we have all of them... */
    if((!type && !id) || !drops) {
        debug(DBG_ERROR, "One or more required monster attributes missing\n");
        rv = -1;
        goto err;
    }
    else if(type && id) {
        debug(DBG_ERROR, "Cannot specify both id and type for monster\n");
        rv = -2;
        goto err;
    }

    /* Make sure the drops value is sane */
    if(!xmlStrcmp(drops, XC"none")) {
        drop = SYLVERANT_QUEST_ENDROP_NONE;
    }
    else if(!xmlStrcmp(drops, XC"norare")) {
        drop = SYLVERANT_QUEST_ENDROP_NORARE;
    }
    else if(!xmlStrcmp(drops, XC"partial")) {
        drop = SYLVERANT_QUEST_ENDROP_PARTIAL;
    }
    else if(!xmlStrcmp(drops, XC"free")) {
        drop = SYLVERANT_QUEST_ENDROP_FREE;
    }
    else if(!xmlStrcmp(drops, XC"default")) {
        drop = def;
    }
    else {
        debug(DBG_ERROR, "Invalid monster drops: %s\n", (char *)drops);
        rv = -3;
        goto err;
    }

    /* Which definition did the user give us? */
    if(type) {
        /* Parse out the type number. */
        errno = 0;
        num = (uint32_t)strtoul((const char *)type, NULL, 0);
        if(errno) {
            debug(DBG_ERROR, "Error parsing monster type \"%s\": %s\n",
                  (const char *)type, strerror(errno));
            rv = -4;
            goto err;
        }

        /* Make space for this type of enemy. */
        count = q->num_monster_types + 1;

        if(!(tmp = realloc(q->monster_types, count * sizeof(mon)))) {
            debug(DBG_ERROR, "Error allocating monster types: %s\n",
                  strerror(errno));
            rv = -5;
            goto err;
        }

        /* Save the new enemy type in the list. */
        q->monster_types = (mon *)tmp;
        q->monster_types[count - 1].key = num;
        q->monster_types[count - 1].value = drop;
        q->monster_types[count - 1].mask = mask;
        q->num_monster_types = count;
    }
    else {
        /* Parse out the ID number. */
        errno = 0;
        num = (uint32_t)strtoul((const char *)id, NULL, 0);
        if(errno) {
            debug(DBG_ERROR, "Error parsing monster id \"%s\": %s\n",
                  (const char *)id, strerror(errno));
            rv = -6;
            goto err;
        }

        /* Make space for this enemy. */
        count = q->num_monster_ids + 1;

        if(!(tmp = realloc(q->monster_ids, count * sizeof(mon)))) {
            debug(DBG_ERROR, "Error allocating monster ids: %s\n",
                  strerror(errno));
            rv = -7;
            goto err;
        }

        /* Save the new enemy in the list. */
        q->monster_ids = (mon *)tmp;
        q->monster_ids[count - 1].key = num;
        q->monster_ids[count - 1].value = drop;
        q->monster_ids[count - 1].mask = mask;
        q->num_monster_ids = count;
    }

err:
    xmlFree(type);
    xmlFree(id);
    xmlFree(drops);
    return rv;
}

static int handle_drops(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *def, *typ;
    int rv = 0;
    uint32_t drop_def = SYLVERANT_QUEST_ENDROP_NORARE;
    uint32_t mask = SYLVERANT_QUEST_ENDROP_SDROPS;

    /* Grab the attribute we're expecting. */
    def = xmlGetProp(n, XC"default");
    typ = xmlGetProp(n, XC"type");

    if(!def) {
        debug(DBG_ERROR, "<drops> tag missing default attribute.\n");
        return -1;
    }

    /* If the type attribute is set, parse it. Otherwise default to server. */
    if(typ) {
        if(!xmlStrcmp(typ, XC"client"))
            mask = SYLVERANT_QUEST_ENDROP_CDROPS;
        else if(!xmlStrcmp(typ, XC"both"))
            mask |= SYLVERANT_QUEST_ENDROP_CDROPS;
        else if(xmlStrcmp(typ, XC"server")) {
            debug(DBG_ERROR, "Invalid type attribute for <drops>: '%s'\n",
                  (char *)typ);
            return -1;
        }
    }

    /* Make sure the default value is sane */
    if(!xmlStrcmp(def, XC"none")) {
        drop_def = SYLVERANT_QUEST_ENDROP_NONE;
    }
    else if(!xmlStrcmp(def, XC"norare")) {
        drop_def = SYLVERANT_QUEST_ENDROP_NORARE;
    }
    else if(!xmlStrcmp(def, XC"partial")) {
        drop_def = SYLVERANT_QUEST_ENDROP_PARTIAL;
    }
    else if(!xmlStrcmp(def, XC"free")) {
        drop_def = SYLVERANT_QUEST_ENDROP_FREE;
    }
    else {
        debug(DBG_ERROR, "Invalid drops default: %s\n", (char *)def);
        rv = -2;
        goto err;
    }

    /* Now that we're done with that, deal with any children of the node */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"monster")) {
            if(handle_monster(n, q, drop_def, mask)) {
                rv = -3;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

err:
    xmlFree(def);
    return rv;
}

static int handle_syncregs(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *def, *list;
    int rv = 0, cnt, ne;
    uint8_t *sr = NULL;
    char *tmp, *tok;
    unsigned long val;
    void *p;

    /* Grab the attributes we're expecting. */
    def = xmlGetProp(n, XC"default");
    list = xmlGetProp(n, XC"list");

    /* The default attribute is required. */
    if(!def) {
        debug(DBG_ERROR, "syncregs requires a default attribute\n");
        rv = -1;
        goto err;
    }

    /* What we do next depends on the default setting... */
    if(!xmlStrcmp(def, "all")) {
        if(list) {
            debug(DBG_ERROR, "syncregs can't have default='all' and list\n");
            rv = -2;
            goto err;
        }

        q->flags |= SYLVERANT_QUEST_SYNC_REGS | SYLVERANT_QUEST_SYNC_ALL;
    }
    else if(!xmlStrcmp(def, "none")) {
        if(!list) {
            debug(DBG_ERROR, "syncregs requires list with default='none'\n");
            rv = -3;
            goto err;
        }

        /* Parse the list... */
        if(!(sr = (uint8_t *)malloc(10))) {
            debug(DBG_ERROR, "Malloc failed!\n");
            rv = -4;
            goto err;
        }

        cnt = 10;
        ne = 0;

        tok = strtok_r((char *)list, " ,;\t\n", &tmp);
        while(tok) {
            errno = 0;
            val = strtoul(tok, NULL, 0);
            if(errno) {
                debug(DBG_ERROR, "Invalid element in syncregs list: %s\n", tok);
                rv = -5;
                goto err;
            }

            if(val > 255) {
                debug(DBG_ERROR, "Invalid element in syncregs list: %d\n", val);
                rv = -6;
                goto err;
            }

            /* If we need more space, double it. */
            if(ne > cnt) {
                p = realloc(sr, cnt << 1);
                if(!p) {
                    debug(DBG_ERROR, "Realloc failed!\n");
                    rv = -7;
                    goto err;
                }

                cnt <<= 1;
                sr = (uint8_t *)p;
            }

            /* Store the parsed value */
            sr[ne++] = (uint8_t)val;

            /* Parse the next one out */
            tok = strtok_r(NULL, " ,;\t\n", &tmp);
        }

        /* Resize the array down -- we don't fret if this fails, because we're
           just making it smaller anyway... */
        p = realloc(sr, ne);
        if(p)
            sr = (uint8_t *)p;

        /* We've parsed the whole list, store it. */
        q->flags |= SYLVERANT_QUEST_SYNC_REGS;
        q->num_sync = ne;
        q->synced_regs = sr;
    }

err:
    if(rv < 0 && sr)
        free(sr);

    xmlFree(list);
    xmlFree(def);
    return rv;
}

static int handle_availability(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *start_str, *end_str;
    struct tm start_tm, end_tm;
    time_t start_val = 0, end_val = 0;
    int rv = 0;

    /* Grab the attributes we're expecting. */
    start_str = xmlGetProp(n, XC"start");
    end_str = xmlGetProp(n, XC"end");

    if(start_str) {
        if(!strptime((const char *)start_str, "%Y-%m-%d %T", &start_tm)) {
            debug(DBG_ERROR, "Cannot parse start time: '%s' on line %hu\n"
                  "Must be of the form YYYY-MM-DD HH:MM:SS\n",
                  (const char *)start_str, n->line);
            rv = -1;
            goto err;
        }

        start_val = timegm(&start_tm);

        if(start_val < 0) {
            debug(DBG_ERROR, "Invalid start time specified on line %hu\n",
                  n->line);
            rv = -2;
            goto err;
        }
    }

    if(end_str) {
        if(!strptime((const char *)end_str, "%Y-%m-%d %T", &end_tm)) {
            debug(DBG_ERROR, "Cannot parse end time: '%s' on line %hu\n"
                  "Must be of the form YYYY-MM-DD HH:MM:SS\n",
                  (const char *)end_str, n->line);
            rv = -3;
            goto err;
        }

        end_val = timegm(&end_tm);

        if(end_val < 0) {
            debug(DBG_ERROR, "Invalid end time specified on line %hu\n",
                  n->line);
            rv = -4;
            goto err;
        }

        if(end_val < start_val) {
            debug(DBG_ERROR, "End time is before start time on line %hu\n",
                  n->line);
            rv = -5;
            goto err;
        }
    }

    q->start_time = (uint64_t)start_val;
    q->end_time = (uint64_t)end_val;

err:
    xmlFree(end_str);
    xmlFree(start_str);
    return rv;
}

static int handle_quest(xmlNode *n, sylverant_quest_category_t *c) {
    xmlChar *name, *prefix, *v1, *v2, *gc, *bb, *ep, *event, *fmt, *id, *sync;
    xmlChar *minpl, *maxpl, *join, *sflag, *sctl, *sdata, *priv;
    int rv = 0, format;
    void *tmp;
    unsigned long episode, id_num, min_pl = 1, max_pl = 4, sf = 0, lc = 0;
    unsigned long ld = 0, sd = 0, sc = 0, privs = 0;
    long event_num;
    sylverant_quest_t *q;
    char *lasts, *token;
    int event_list = 0;

    /* Grab the attributes we're expecting */
    name = xmlGetProp(n, XC"name");
    prefix = xmlGetProp(n, XC"prefix");
    v1 = xmlGetProp(n, XC"v1");
    v2 = xmlGetProp(n, XC"v2");
    gc = xmlGetProp(n, XC"gc");
    bb = xmlGetProp(n, XC"bb");
    ep = xmlGetProp(n, XC"episode");
    event = xmlGetProp(n, XC"event");
    fmt = xmlGetProp(n, XC"format");
    id = xmlGetProp(n, XC"id");
    minpl = xmlGetProp(n, XC"minpl");
    maxpl = xmlGetProp(n, XC"maxpl");
    sync = xmlGetProp(n, XC"sync");
    join = xmlGetProp(n, XC"joinable");
    sflag = xmlGetProp(n, XC"sflag");
    sdata = xmlGetProp(n, XC"datareg");
    sctl = xmlGetProp(n, XC"ctlreg");
    priv = xmlGetProp(n, XC"privileges");

    /* Make sure we have all of them... */
    if(!name || !prefix || !v1 || !ep || !event || !fmt || !id) {
        debug(DBG_ERROR, "One or more required quest attributes missing\n");
        rv = -1;
        goto err;
    }

    /* Make sure the episode is sane */
    episode = strtoul((const char *)ep, NULL, 0);

    if(episode < 1 || episode > 4) {
        debug(DBG_ERROR, "Invalid episode given: %s\n", ep);
        rv = -2;
        goto err;
    }

    /* Make sure the format is sane */
    if(!xmlStrcmp(fmt, XC"qst")) {
        format = SYLVERANT_QUEST_QST;
    }
    else if(!xmlStrcmp(fmt, XC"bindat")) {
        format = SYLVERANT_QUEST_BINDAT;
    }
    else if(!xmlStrcmp(fmt, XC"ubindat")) {
        format = SYLVERANT_QUEST_UBINDAT;
    }
    else {
        debug(DBG_ERROR, "Invalid format given for quest: %s\n", fmt);
        rv = -3;
        goto err;
    }

    /* Make sure the event list is sane */
    token = strtok_r((char *)event, ", ", &lasts);

    if(!token) {
        debug(DBG_ERROR, "Invalid event given for quest: %s\n", event);
        rv = -4;
        goto err;
    }

    while(token) {
        /* Parse the token */
        errno = 0;
        event_num = strtol(token, NULL, 0);

        if(errno || event_num < -1 || event_num > 14) {
            debug(DBG_ERROR, "Invalid token in event: %s\n", token);
            rv = -9;
            goto err;
        }

        /* Make sure its valid now */
        if(event_list == -1) {
            debug(DBG_ERROR, "Invalid event number specified after -1: %s\n",
                  token);
            rv = -10;
            goto err;
        }
        else if(event_list != 0 && event_num == -1) {
            debug(DBG_ERROR, "Invalid to specify -1 after an event\n");
            rv = -11;
            goto err;
        }

        /* Set the bit for the specified event */
        if(event_num == -1) {
            event_list = -1;
        }
        else {
            event_list |= (1 << event_num);
        }

        /* Read the next event in, if any */
        token = strtok_r(NULL, ", ", &lasts);
    }

    /* Make sure the id is sane */
    errno = 0;
    id_num = strtoul((const char *)id, NULL, 0);

    if(errno) {
        debug(DBG_ERROR, "Invalid ID given for quest: %s\n", (const char *)id);
        rv = -5;
        goto err;
    }

    /* Make sure the min/max players count is sane */
    if(minpl) {
        min_pl = strtoul((const char *)minpl, NULL, 0);

        if(min_pl < 1 || min_pl > 4) {
            debug(DBG_ERROR, "Invalid minimum players given: %s\n",
                  (const char *)minpl);
            rv = -9;
            goto err;
        }
    }

    if(maxpl) {
        max_pl = strtoul((const char *)maxpl, NULL, 0);

        if(max_pl < 1 || max_pl > 4 || max_pl < min_pl) {
            debug(DBG_ERROR, "Invalid maximum players given: %s\n",
                  (const char *)maxpl);
            rv = -10;
            goto err;
        }
    }

    /* If the flag stuff is set, parse it and check for sanity. */
    if(sflag) {
        errno = 0;
        sf = strtoul((const char *)sflag, NULL, 0);

        if(errno || sf > 255) {
            debug(DBG_ERROR, "Invalid sflag given: %s\n", (const char *)sflag);
            rv = -11;
            goto err;
        }
    }

    /* Check if this quest uses server data function calls */
    if((sctl && !sdata) || (sdata && !sctl)) {
        debug(DBG_ERROR, "Must give both of datareg/ctlreg (or neither)\n");
        rv = -12;
        goto err;
    }

    if(sctl) {
        errno = 0;
        sc = strtoul((const char *)sctl, NULL, 0);

        if(errno || sc > 255) {
            debug(DBG_ERROR, "Invalid ctlreg given: %s\n",
                  (const char *)sctl);
            rv = -13;
            goto err;
        }

        sd = strtoul((const char *)sdata, NULL, 0);

        if(errno || sd > 255) {
            debug(DBG_ERROR, "Invalid datareg given: %s\n",
                  (const char *)sdata);
            rv = -14;
            goto err;
        }
    }

    if(priv) {
        /* Make sure the privilege value is sane */
        errno = 0;
        privs = strtoul((const char *)priv, NULL, 0);

        if(errno) {
            debug(DBG_ERROR, "Invalid privilege value for quest: %s\n",
                  (const char *)priv);
            rv = -16;
            goto err;
        }
    }

    /* Allocate space for the quest */
    tmp = realloc(c->quests, (c->quest_count + 1) * sizeof(sylverant_quest_t*));

    if(!tmp) {
        debug(DBG_ERROR, "Couldn't allocate space for quest in array\n");
        perror("realloc");
        rv = -6;
        goto err;
    }

    c->quests = (sylverant_quest_t **)tmp;

    q = (sylverant_quest_t *)ref_alloc(sizeof(sylverant_quest_t), &quest_dtor);
    if(!q) {
        debug(DBG_ERROR, "Couldn't allocate space for quest\n");
        perror("ref_alloc");
        rv = -6;
        goto err;
    }

    c->quests[c->quest_count++] = q;

    /* Clear the quest out */
    memset(q, 0, sizeof(sylverant_quest_t));

    /* Copy over what we have so far */
    q->qid = (uint32_t)id_num;
    q->privileges = privs;
    q->episode = (int)episode;
    q->event = (int)event_list;
    q->format = (int)format;

    strncpy(q->name, (const char *)name, 31);
    q->name[31] = '\0';
    q->prefix = (char *)prefix;

    /* Fill in the versions */
    if(!xmlStrcmp(v1, XC"true"))
        q->versions |= SYLVERANT_QUEST_V1;

    if(!xmlStrcmp(v2, XC"true"))
        q->versions |= SYLVERANT_QUEST_V2;

    if(!xmlStrcmp(gc, XC"true"))
        q->versions |= SYLVERANT_QUEST_GC;

    if(!xmlStrcmp(bb, XC"true"))
        q->versions |= SYLVERANT_QUEST_BB;

    q->min_players = min_pl;
    q->max_players = max_pl;

    if(sync && !xmlStrcmp(sync, XC"true"))
        q->sync = 1;

    if(sflag) {
        q->flags |= SYLVERANT_QUEST_FLAG16;
        q->server_flag16_reg = (uint8_t)sf;
    }

    if(sdata) {
        q->flags |= SYLVERANT_QUEST_DATAFL;
        q->server_data_reg = (uint8_t)sd;
        q->server_ctl_reg = (uint8_t)sc;
    }

    if(join && !xmlStrcmp(join, XC"true"))
        q->flags |= SYLVERANT_QUEST_JOINABLE;

    /* Now that we're done with that, deal with any children of the node */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"long")) {
            if(handle_long(n, q)) {
                rv = -7;
                goto err;
            }
        }
        else if(!xmlStrcmp(n->name, XC"short")) {
            if(handle_short(n, q)) {
                rv = -8;
                goto err;
            }
        }
        else if(!xmlStrcmp(n->name, XC"drops")) {
            if(handle_drops(n, q)) {
                rv = -9;
                goto err;
            }
        }
        else if(!xmlStrcmp(n->name, XC"syncregs")) {
            if(handle_syncregs(n, q)) {
                rv = -15;
                goto err;
            }
        }
        else if(!xmlStrcmp(n->name, XC"availability")) {
            if(handle_availability(n, q)) {
                rv = -16;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

err:
    xmlFree(name);
    xmlFree(v1);
    xmlFree(v2);
    xmlFree(gc);
    xmlFree(bb);
    xmlFree(ep);
    xmlFree(event);
    xmlFree(fmt);
    xmlFree(id);
    xmlFree(minpl);
    xmlFree(maxpl);
    xmlFree(sync);
    xmlFree(join);
    xmlFree(sflag);
    xmlFree(sctl);
    xmlFree(sdata);
    xmlFree(priv);
    return rv;
}

static int handle_description(xmlNode *n, sylverant_quest_category_t *c) {
    xmlChar *desc;

    /* Grab the description from the node */
    if((desc = xmlNodeListGetString(n->doc, n->children, 1))) {
        strncpy(c->desc, (const char *)desc, 111);
        c->desc[111] = '\0';
        xmlFree(desc);
    }

    return 0;
}

static int handle_category(xmlNode *n, sylverant_quest_list_t *l) {
    xmlChar *name, *type, *eps, *priv;
    char *token, *lasts;
    int rv = 0;
    uint32_t type_num;
    uint32_t episodes = SYLVERANT_QUEST_EP1 | SYLVERANT_QUEST_EP2 |
        SYLVERANT_QUEST_EP4;
    uint32_t privs = 0;
    void *tmp;
    sylverant_quest_category_t *cat;
    int epnum;

    /* Grab the attributes we're expecting */
    name = xmlGetProp(n, XC"name");
    type = xmlGetProp(n, XC"type");
    eps = xmlGetProp(n, XC"episodes");
    priv = xmlGetProp(n, XC"privileges");

    /* Make sure we have both of them... */
    if(!name || !type) {
        debug(DBG_ERROR, "Name or type not given for category\n");
        rv = -1;
        goto err;
    }

    /* Make sure the type is sane */
    if(!xmlStrcmp(type, XC"normal")) {
        type_num = SYLVERANT_QUEST_NORMAL;
    }
    else if(!xmlStrcmp(type, XC"battle")) {
        type_num = SYLVERANT_QUEST_BATTLE;
    }
    else if(!xmlStrcmp(type, XC"challenge")) {
        type_num = SYLVERANT_QUEST_CHALLENGE;
    }
    else if(!xmlStrcmp(type, XC"government")) {
        type_num = SYLVERANT_QUEST_GOVERNMENT;
    }
    else if(!xmlStrcmp(type, XC"debug")) {
        type_num = SYLVERANT_QUEST_NORMAL | SYLVERANT_QUEST_DEBUG;
    }
    else {
        debug(DBG_ERROR, "Invalid category type: %s\n", (char *)type);
        rv = -2;
        goto err;
    }

    if(priv) {
        /* Make sure the privilege value is sane */
        errno = 0;
        privs = strtoul((const char *)priv, NULL, 0);

        if(errno) {
            debug(DBG_ERROR, "Invalid privilege value for category: %s\n",
                  (const char *)priv);
            rv = -8;
            goto err;
        }
    }

    /* Is there an episode list specified? */
    if(eps) {
        /* Parse it. */
        token = strtok_r((char *)eps, ", ", &lasts);
        episodes = 0;

        if(!token) {
            debug(DBG_ERROR, "Invalid episodes given for category: %s\n",
                  (char *)eps);
            rv = -6;
            goto err;
        }

        while(token) {
            /* Parse the token */
            errno = 0;
            epnum = (int)strtol(token, NULL, 0);

            if(errno || (epnum != 1 && epnum != 2 && epnum != 4)) {
                debug(DBG_ERROR, "Invalid token in episodes: %s\n", token);
                rv = -7;
                goto err;
            }

            /* Set the bit for the specified episode */
            episodes |= epnum;

            /* Read the next number in, if any */
            token = strtok_r(NULL, ", ", &lasts);
        }
    }

    /* Allocate space for the category */
    tmp = realloc(l->cats, (l->cat_count + 1) *
                  sizeof(sylverant_quest_category_t));

    if(!tmp) {
        debug(DBG_ERROR, "Couldn't allocate space for category\n");
        perror("realloc");
        rv = -3;
        goto err;
    }

    l->cats = (sylverant_quest_category_t *)tmp;
    cat = l->cats + l->cat_count++;

    /* Clear the category out */
    memset(cat, 0, sizeof(sylverant_quest_category_t));

    /* Copy over what we have so far */
    cat->type = type_num;
    cat->episodes = episodes;
    cat->privileges = privs;

    strncpy(cat->name, (const char *)name, 31);
    cat->name[31] = '\0';

    /* Now that we're done with that, deal with any children of the node */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"description")) {
            if(handle_description(n, cat)) {
                rv = -4;
                goto err;
            }
        }
        else if(!xmlStrcmp(n->name, XC"quest")) {
            if(handle_quest(n, cat)) {
                rv = -5;
                goto err;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

err:
    xmlFree(priv);
    xmlFree(name);
    xmlFree(type);
    xmlFree(eps);
    return rv;
}

int sylverant_quests_read(const char *filename, sylverant_quest_list_t *rv) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    int irv = 0;

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_quest_list_t));

    /* Make sure the file exists and can be read, otherwise quietly bail out */
    if(access(filename, R_OK)) {
        return -1;
    }

    /* Create an XML Parsing context */
    cxt = xmlNewParserCtxt();
    if(!cxt) {
        debug(DBG_ERROR, "Couldn't create parsing context for config\n");
        irv = -2;
        goto err;
    }

    /* Open the configuration file for reading. */
    doc = xmlReadFile(filename, NULL, XML_PARSE_DTDVALID);

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

    /* If we've gotten this far, we have a valid document, go through and read
       everything contained within... */
    n = xmlDocGetRootElement(doc);

    if(!n) {
        debug(DBG_WARN, "Empty config document\n");
        irv = -5;
        goto err_doc;
    }

    /* Make sure the config looks sane. */
    if(xmlStrcmp(n->name, XC"quests")) {
        debug(DBG_WARN, "Quest List does not appear to be the right type\n");
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
        else if(!xmlStrcmp(n->name, XC"category")) {
            if(handle_category(n, rv)) {
                irv = -7;
                goto err_clean;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

    /* Cleanup/error handling below... */
err_clean:
    if(irv < 0) {
        sylverant_quests_destroy(rv);
    }

err_doc:
    xmlFreeDoc(doc);
err_cxt:
    xmlFreeParserCtxt(cxt);
err:
    return irv;
}

static void quest_dtor(void *o) {
    sylverant_quest_t *q = (sylverant_quest_t *)o;

    if(!q)
        return;

    xmlFree(q->long_desc);
    xmlFree(q->prefix);
    free(q->monster_ids);
    free(q->monster_types);
    free(q->synced_regs);
}

void sylverant_quests_destroy(sylverant_quest_list_t *list) {
    int i, j;
    sylverant_quest_category_t *cat;

    for(i = 0; i < list->cat_count; ++i) {
        cat = &list->cats[i];

        for(j = 0; j < cat->quest_count; ++j) {
            ref_release(cat->quests[j]);
        }

        /* Free the list of quests. */
        free(cat->quests);
    }

    /* Free the list of categories, and we're done. */
    free(list->cats);
    list->cats = NULL;
    list->cat_count = 0;
}
