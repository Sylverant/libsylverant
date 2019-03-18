/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011, 2014, 2015, 2018, 2019 Lawrence Sebald

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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "sylverant/quest.h"
#include "sylverant/debug.h"

#ifndef LIBXML_TREE_ENABLED
#error You must have libxml2 with tree support built-in.
#endif

#define XC (const xmlChar *)

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

static int handle_monster(xmlNode *n, sylverant_quest_t *q, uint32_t def) {
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
        debug(DBG_ERROR, "One or more required monster attributes missing %p %p %p\n", type, id, drops);
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
        q->num_monster_ids = count;
    }

err:
    xmlFree(type);
    xmlFree(id);
    xmlFree(drops);
    return rv;
}

static int handle_drops(xmlNode *n, sylverant_quest_t *q) {
    xmlChar *def;
    int rv = 0;
    uint32_t drop_def = SYLVERANT_QUEST_ENDROP_NORARE;

    /* Grab the attribute we're expecting. */
    def = xmlGetProp(n, XC"default");

    if(!def) {
        debug(DBG_ERROR, "<drops> tag missing default attribute.\n");
        return -1;
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
            if(handle_monster(n, q, drop_def)) {
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

static int handle_quest(xmlNode *n, sylverant_quest_category_t *c) {
    xmlChar *name, *prefix, *v1, *v2, *gc, *bb, *ep, *event, *fmt, *id, *sync;
    xmlChar *minpl, *maxpl, *join, *sflag, *lctl, *ldat;
    int rv = 0, format;
    void *tmp;
    unsigned long episode, id_num, min_pl = 1, max_pl = 4, sf = 0, lc = 0;
    unsigned long ld = 0;
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
    lctl = xmlGetProp(n, XC"lflagctl");
    ldat = xmlGetProp(n, XC"lflagdat");

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

    if((lctl && !ldat) || (ldat && !lctl)) {
        debug(DBG_ERROR, "Must give both of lflagdat/lflagctl (or neither)\n");
        rv = -12;
        goto err;
    }

    if(lctl) {
        errno = 0;
        lc = strtoul((const char *)lctl, NULL, 0);

        if(errno || lc > 255) {
            debug(DBG_ERROR, "Invalid lflagctl given: %s\n",
                  (const char *)lctl);
            rv = -13;
            goto err;
        }

        ld = strtoul((const char *)ldat, NULL, 0);

        if(errno || lc > 255) {
            debug(DBG_ERROR, "Invalid lflagdat given: %s\n",
                  (const char *)ldat);
            rv = -14;
            goto err;
        }
    }

    /* Allocate space for the quest */
    tmp = realloc(c->quests, (c->quest_count + 1) * sizeof(sylverant_quest_t));

    if(!tmp) {
        debug(DBG_ERROR, "Couldn't allocate space for quest\n");
        perror("realloc");
        rv = -6;
        goto err;
    }

    c->quests = (sylverant_quest_t *)tmp;
    q = c->quests + c->quest_count++;

    /* Clear the quest out */
    memset(q, 0, sizeof(sylverant_quest_t));

    /* Copy over what we have so far */
    q->qid = (uint32_t)id_num;
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

    if(lctl) {
        q->flags |= SYLVERANT_QUEST_FLAG32;
        q->server_flag32_ctl = (uint8_t)lc;
        q->server_flag32_dat = (uint8_t)ld;
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
    xmlFree(lctl);
    xmlFree(ldat);
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
    xmlChar *name, *type, *eps;
    char *token, *lasts;
    int rv = 0;
    uint32_t type_num;
    uint32_t episodes = SYLVERANT_QUEST_EP1 | SYLVERANT_QUEST_EP2 |
        SYLVERANT_QUEST_EP4;
    void *tmp;
    sylverant_quest_category_t *cat;
    int epnum;

    /* Grab the attributes we're expecting */
    name = xmlGetProp(n, XC"name");
    type = xmlGetProp(n, XC"type");
    eps = xmlGetProp(n, XC"episodes");

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

void sylverant_quests_destroy(sylverant_quest_list_t *list) {
    int i, j;
    sylverant_quest_category_t *cat;
    sylverant_quest_t *quest;

    for(i = 0; i < list->cat_count; ++i) {
        cat = &list->cats[i];

        for(j = 0; j < cat->quest_count; ++j) {
            quest = &cat->quests[j];

            /* Free each malloced item in the quest. */
            xmlFree(quest->long_desc);
            xmlFree(quest->prefix);
            free(quest->monster_ids);
            free(quest->monster_types);
        }

        /* Free the list of quests. */
        free(cat->quests);
    }

    /* Free the list of categories, and we're done. */
    free(list->cats);
    list->cats = NULL;
    list->cat_count = 0;
}
