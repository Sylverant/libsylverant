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
        strncpy(q->desc, desc, 31);
        q->desc[31] = '\0';
        xmlFree(desc);
    }

    return 0;
}

static int handle_quest(xmlNode *n, sylverant_quest_category_t *c) {
    xmlChar *name, *prefix, *v1, *v2, *gc, *bb, *ep, *event, *fmt, *id;
    int rv = 0, format;
    void *tmp;
    unsigned long episode, id_num;
    long event_num;
    sylverant_quest_t *q;

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

    /* Make sure we have all of them... */
    if(!name || !prefix || !v1 || !v2 || !gc || !bb || !ep || !event || !fmt ||
       !id) {
        debug(DBG_ERROR, "One or more required quest attributes missing\n");
        rv = -1;
        goto err;
    }

    /* Make sure the episode is sane */
    episode = strtoul(ep, NULL, 0);

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
    else {
        debug(DBG_ERROR, "Invalid format given for quest: %s\n", fmt);
        rv = -3;
        goto err;
    }

    /* Make sure the event is sane */
    errno = 0;
    event_num = strtol(event, NULL, 0);

    if(errno || event_num < -1 || event_num > 14) {
        debug(DBG_ERROR, "Invalid event given for quest: %s\n", event);
        rv = -4;
        goto err;
    }

    /* Make sure the id is sane */
    errno = 0;
    id_num = strtoul(id, NULL, 0);

    if(errno) {
        debug(DBG_ERROR, "Invalid ID given for quest: %s\n", id);
        rv = -5;
        goto err;
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
    q->event = (int)event_num;
    q->format = (int)format;

    strncpy(q->name, name, 31);
    q->name[31] = '\0';
    q->prefix = prefix;

    /* Fill in the versions */
    if(!xmlStrcmp(v1, XC"true")) {
        q->versions |= SYLVERANT_QUEST_V1;
    }
    if(!xmlStrcmp(v2, XC"true")) {
        q->versions |= SYLVERANT_QUEST_V2;
    }
    if(!xmlStrcmp(gc, XC"true")) {
        q->versions |= SYLVERANT_QUEST_GC;
    }
    if(!xmlStrcmp(bb, XC"true")) {
        q->versions |= SYLVERANT_QUEST_BB;
    }

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
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

err:
    xmlFree(name);
    xmlFree(prefix);
    xmlFree(v1);
    xmlFree(v2);
    xmlFree(gc);
    xmlFree(bb);
    xmlFree(ep);
    xmlFree(event);
    xmlFree(fmt);
    xmlFree(id);
    return rv;
}

static int handle_description(xmlNode *n, sylverant_quest_category_t *c) {
    xmlChar *desc;

    /* Grab the description from the node */
    if((desc = xmlNodeListGetString(n->doc, n->children, 1))) {
        strncpy(c->desc, desc, 111);
        c->desc[111] = '\0';
        xmlFree(desc);
    }

    return 0;
}

static int handle_category(xmlNode *n, sylverant_quest_list_t *l) {
    xmlChar *name, *type;
    int rv = 0;
    uint32_t type_num;
    void *tmp;
    sylverant_quest_category_t *cat;

    /* Grab the attributes we're expecting */
    name = xmlGetProp(n, XC"name");
    type = xmlGetProp(n, XC"type");

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
    else {
        debug(DBG_ERROR, "Invalid category type: %s\n", (char *)type);
        rv = -2;
        goto err;
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
    strncpy(cat->name, name, 31);
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
    return rv;
}

int sylverant_quests_read(const char *filename, sylverant_quest_list_t *rv) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    int irv = 0;

    /* Make sure the file exists and can be read, otherwise quietly bail out */
    if(access(filename, R_OK)) {
        return -1;
    }

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_quest_list_t));

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
        }

        /* Free the list of quests. */
        free(cat->quests);
    }

    /* Free the list of categories, and we're done. */
    free(list->cats);
    list->cats = NULL;
    list->cat_count = 0;
}
