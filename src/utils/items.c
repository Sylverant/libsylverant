/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2010, 2011 Lawrence Sebald

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
#include <limits.h>
#include <unistd.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "sylverant/items.h"
#include "sylverant/debug.h"

#ifndef LIBXML_TREE_ENABLED
#error You must have libxml2 with tree support built-in.
#endif

#define XC (const xmlChar *)

/* Lazy, lazy, lazy */
typedef int8_t s8;

#define FULL_ATTR_VALID 0x1FFFFFFFFFFULL

/* List of valid weapon attributes. */
static const char *weapon_attrs[Weapon_Attr_MAX + 1] = {
    "None",
    "Draw",
    "Drain",
    "Fill",
    "Gush",
    "Heart",
    "Mind",
    "Soul",
    "Geist",
    "Master's",
    "Lord's",
    "King's",
    "Charge",
    "Spirit",
    "Berserk",
    "Ice",
    "Frost",
    "Freeze",
    "Blizzard",
    "Bind",
    "Hold",
    "Seize",
    "Arrest",
    "Heat",
    "Fire",
    "Flame",
    "Burning",
    "Shock",
    "Thunder",
    "Storm",
    "Tempest",
    "Dim",
    "Shadow",
    "Dark",
    "Hell",
    "Panic",
    "Riot",
    "Havoc",
    "Chaos",
    "Devil's",
    "Demon's"
};

#define NUM_PBS 8

/* List of Mag Photon Blasts */
static const char *mag_pbs[NUM_PBS] = {
    "Farlla", "Estlla", "Golla", "Pilla", "Leilla", "Mylla+Youlla", /* Legit */
    "bad1", "bad2" /* Not legit */
};

#define NUM_COLORS 16

/* List of Mag colors */
static const char *mag_colors[NUM_COLORS] = {
    "red", "blue", "yellow", "green", "purple", "darkpurple", "white", "cyan",
    "brown", "black", "c10", "c11", "c12", "c13", "c14", "c15"
};

static int handle_pbs(xmlNode *n, uint8_t *c, uint8_t *r, uint8_t *l) {
    xmlChar *pos, *pbs;
    char *lasts, *tok;
    int i, rv = 0;
    uint8_t *valid;

    /* Grab the attributes */
    pos = xmlGetProp(n, XC"pos");
    pbs = xmlGetProp(n, XC"pbs");

    if(!pos || !pbs) {
        debug(DBG_ERROR, "pbs tag without required attributes\n");
        rv = -1;
        goto err;
    }

    /* Figure out what list to look at */
    if(!xmlStrcmp(pos, "center")) {
        valid = c;
    }
    else if(!xmlStrcmp(pos, "right")) {
        valid = r;
    }
    else if(!xmlStrcmp(pos, "left")) {
        valid = l;
    }
    else {
        debug(DBG_ERROR, "Invalid position for pbs tag: %s\n", pos);
        rv = -2;
        goto err;
    }

    /* Tokenize the pbs string */
    tok = strtok_r((char *)pbs, ", ", &lasts);

    *valid = 0xFF;

    /* Go through any PBs in the disallow list */
    while(tok) {
        /* Look through the list of PBs for what we have */
        for(i = 0; i < NUM_PBS; ++i) {
            if(!strcmp(mag_pbs[i], tok)) {
                *valid &= ~(1 << i);
                break;
            }
        }

        /* If we didn't find the PB, die */
        if(i == NUM_PBS) {
            debug(DBG_ERROR, "Invalid pb: %s\n", tok);
            rv = -3;
            goto err;
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

err:
    xmlFree(pos);
    xmlFree(pbs);
    return rv;
}

static int handle_colors(xmlNode *n, uint16_t *valid) {
    xmlChar *colors;
    char *lasts, *tok;
    int i;

    /* Attempt to grab the color list */
    colors = xmlGetProp(n, XC"disallow");

    if(!colors) {
        debug(DBG_ERROR, "colors tag with no disallow list\n");
        return -1;
    }

    /* Tokenize the string... */
    tok = strtok_r((char *)colors, ", ", &lasts);

    *valid = 0xFFFF;

    /* Go through any colors in the disallow list */
    while(tok) {
        /* Look through the list of colors for what we have */
        for(i = 0; i < NUM_COLORS; ++i) {
            if(!strcmp(mag_colors[i], tok)) {
                *valid &= ~(1 << i);
                break;
            }
        }

        /* If we didn't find the color, die */
        if(i == NUM_COLORS) {
            debug(DBG_ERROR, "Invalid color: %s\n", tok);
            xmlFree(colors);
            return -1;
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

    /* Clean up the string, since we're done */
    xmlFree(colors);

    return 0;
}

static int handle_attributes(xmlNode *n, uint64_t *valid) {
    xmlChar *attr;
    char *lasts, *tok;
    int i;

    /* Attempt to grab the attribute list */
    attr = xmlGetProp(n, XC"disallow");

    if(!attr) {
        debug(DBG_ERROR, "attributes tag with no disallow list\n");
        return -1;
    }

    /* Tokenize the string... */
    tok = strtok_r((char *)attr, ", ", &lasts);

    /* Go through any attributes in the disallow list */
    while(tok) {
        /* Look through the list of attributes for what we have */
        for(i = 0; i <= Weapon_Attr_MAX; ++i) {
            if(!strcmp(weapon_attrs[i], tok)) {
                *valid &= ~(1 << i);
                break;
            }
        }

        /* If we didn't find the attribute, die */
        if(i > Weapon_Attr_MAX) {
            debug(DBG_ERROR, "Invalid attribute: %s\n", tok);
            xmlFree(attr);
            return -1;
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

    /* Clean up the string, since we're done */
    xmlFree(attr);
    
    return 0;
}

static int handle_common_tag(xmlNode *n, sylverant_item_t *i) {
    int rv = 1;

    if(!xmlStrcmp(n->name, XC"auto_reject")) {
        i->auto_reject = 1;
    }
    else if(!xmlStrcmp(n->name, XC"reject_max")) {
        i->reject_max = 1;
    }
    else if(!xmlStrcmp(n->name, XC"versions")) {
        xmlChar *v1, *v2, *gc;

        v1 = xmlGetProp(n, XC"v1");
        v2 = xmlGetProp(n, XC"v2");
        gc = xmlGetProp(n, XC"gc");

        if(!v1 || !v2 || !gc) {
            debug(DBG_ERROR, "Required attribute of versions not found\n");
            rv = 0;
            goto done_vers;
        }

        /* Check each value */
        if(!xmlStrcmp(v1, XC"true")) {
            i->versions |= ITEM_VERSION_V1;
        }
        else if(xmlStrcmp(v1, XC"false")) {
            debug(DBG_ERROR, "Invalid value for attribute v1: %s\n",
                  (char *)v1);
            rv = 0;
            goto done_vers;
        }

        if(!xmlStrcmp(v2, XC"true")) {
            i->versions |= ITEM_VERSION_V2;
        }
        else if(xmlStrcmp(v2, XC"false")) {
            debug(DBG_ERROR, "Invalid value for attribute v2: %s\n",
                  (char *)v2);
            rv = 0;
            goto done_vers;
        }

        if(!xmlStrcmp(gc, XC"true")) {
            i->versions |= ITEM_VERSION_GC;
        }
        else if(xmlStrcmp(gc, XC"false")) {
            debug(DBG_ERROR, "Invalid value for attribute gc: %s\n",
                  (char *)gc);
            rv = 0;
            goto done_vers;
        }

done_vers:
        xmlFree(v1);
        xmlFree(v2);
        xmlFree(gc);
    }
    else {
        rv = 0;
    }

    return rv;
}

static int handle_max(xmlNode *n, int *max, int *min) {
    xmlChar *val;
    int rv = 0;

    /* Grab the attributes */
    if((val = xmlGetProp(n, XC"max"))) {
        errno = 0;
        *max = (int)strtol((char *)val, NULL, 0);

        if(errno) {
            debug(DBG_ERROR, "Invalid value for max: %s\n", (char *)max);
            rv = -1;
        }

        xmlFree(val);
    }

    if((val = xmlGetProp(n, XC"min"))) {
        errno = 0;
        *min = (int)strtol((char *)val, NULL, 0);

        if(errno) {
            debug(DBG_ERROR, "Invalid value for min: %s\n", (char *)min);
            rv = -1;
        }

        xmlFree(val);
    }

    return rv;
}

static int handle_weapon(xmlNode *n, sylverant_weapon_t *w) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"grind")) {
            if(handle_max(n, &w->max_grind, &w->min_grind)) {
                return -1;
            }
        }
        else if(!xmlStrcmp(n->name, XC"percents")) {
            if(handle_max(n, &w->max_percents, &w->min_percents)) {
                return -2;
            }
        }
        else if(!xmlStrcmp(n->name, XC"attributes")) {
            if(handle_attributes(n, &w->valid_attrs)) {
                return -3;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)w)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -4;
        }

        n = n->next;
    }
}

static int handle_frame(xmlNode *n, sylverant_frame_t *f) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"slots")) {
            if(handle_max(n, &f->max_slots, &f->min_slots)) {
                return -1;
            }
        }
        else if(!xmlStrcmp(n->name, XC"dfp")) {
            if(handle_max(n, &f->max_dfp, &f->min_dfp)) {
                return -2;
            }
        }
        else if(!xmlStrcmp(n->name, XC"evp")) {
            if(handle_max(n, &f->max_evp, &f->min_evp)) {
                return -3;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)f)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -4;
        }

        n = n->next;
    }
}

static int handle_barrier(xmlNode *n, sylverant_barrier_t *b) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"dfp")) {
            if(handle_max(n, &b->max_dfp, &b->min_dfp)) {
                return -1;
            }
        }
        else if(!xmlStrcmp(n->name, XC"evp")) {
            if(handle_max(n, &b->max_evp, &b->min_evp)) {
                return -2;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)b)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -3;
        }

        n = n->next;
    }
}

static int handle_unit(xmlNode *n, sylverant_unit_t *u) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"plus")) {
            if(handle_max(n, &u->max_plus, &u->min_plus)) {
                return -1;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)u)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -2;
        }

        n = n->next;
    }
}

static int handle_mag(xmlNode *n, sylverant_mag_t *m) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"level")) {
            if(handle_max(n, &m->max_level, &m->min_level)) {
                return -1;
            }
        }
        else if(!xmlStrcmp(n->name, XC"def")) {
            if(handle_max(n, &m->max_def, &m->min_def)) {
                return -2;
            }
        }
        else if(!xmlStrcmp(n->name, XC"pow")) {
            if(handle_max(n, &m->max_pow, &m->min_pow)) {
                return -3;
            }
        }
        else if(!xmlStrcmp(n->name, XC"dex")) {
            if(handle_max(n, &m->max_dex, &m->min_dex)) {
                return -4;
            }
        }
        else if(!xmlStrcmp(n->name, XC"mind")) {
            if(handle_max(n, &m->max_mind, &m->min_mind)) {
                return -5;
            }
        }
        else if(!xmlStrcmp(n->name, XC"synchro")) {
            if(handle_max(n, &m->max_synchro, &m->min_synchro)) {
                return -6;
            }
        }
        else if(!xmlStrcmp(n->name, XC"iq")) {
            if(handle_max(n, &m->max_iq, &m->min_iq)) {
                return -7;
            }
        }
        else if(!xmlStrcmp(n->name, "pbs")) {
            if(handle_pbs(n, &m->allowed_cpb, &m->allowed_rpb,
                          &m->allowed_lpb)) {
                return -8;
            }
        }
        else if(!xmlStrcmp(n->name, "colors")) {
            if(handle_colors(n, &m->allowed_colors)) {
                return -9;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)m)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -10;
        }

        n = n->next;
    }
}

static int handle_tool(xmlNode *n, sylverant_tool_t *t) {
    int rv = 0;

    /* Look at the node's children... */
    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"stack")) {
            if(handle_max(n, &t->max_stack, &t->min_stack)) {
                return -1;
            }
        }
        else if(handle_common_tag(n, (sylverant_item_t *)t)) {
            n = n->next;
            continue;
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
            return -2;
        }

        n = n->next;
    }
}

static int handle_item(xmlNode *n, sylverant_limits_t *l, int swap) {
    xmlChar *code_str;
    uint32_t code;
    sylverant_item_t *cur_item;

    /* Attempt to grab the item code */
    code_str = xmlGetProp(n, XC"code");

    if(!code_str) {
        debug(DBG_ERROR, "item tag with no item code\n");
        return -1;
    }

    /* Attempt to parse out the code from the string */
    errno = 0;
    code = (uint32_t)strtoul((char *)code_str, NULL, 16);
    if(errno) {
        debug(DBG_ERROR, "Invalid item code: %s\n", (char *)code_str);
        xmlFree(code_str);
        return -2;
    }

    if(swap) {
        code = ((code >> 24) & 0xFF) | ((code >> 8) & 0xFF00) |
            ((code & 0xFF00) << 8) | ((code & 0xFF) << 24);
    }

    /* we're done with this now... */
    xmlFree(code_str);

    /* Now that we have the item code, we can figure out what type of item we
       actually are allocating. */
    switch(code & 0xFF) {
        case ITEM_TYPE_WEAPON:
        {
            sylverant_weapon_t *w;

            w = (sylverant_weapon_t *)malloc(sizeof(sylverant_weapon_t));
            if(!w) {
                debug(DBG_ERROR, "Couldn't allocate space for item\n");
                perror("malloc");
                return -3;
            }

            memset(w, 0xFF, sizeof(sylverant_weapon_t));

            cur_item = (sylverant_item_t *)w;
            w->base.item_code = code;
            w->base.auto_reject = 0;
            w->base.reject_max = 0;
            w->base.versions = 0;
            w->valid_attrs = FULL_ATTR_VALID;
            TAILQ_INSERT_TAIL(l->weapons, cur_item, qentry);

            if(handle_weapon(n, w)) {
                return -4;
            }

            break;
        }

        case ITEM_TYPE_GUARD:
            switch((code >> 8) & 0xFF) {
                case ITEM_SUBTYPE_FRAME:
                {
                    sylverant_frame_t *f;

                    f = (sylverant_frame_t *)malloc(sizeof(sylverant_frame_t));
                    if(!f) {
                        debug(DBG_ERROR, "Couldn't allocate space for item\n");
                        perror("malloc");
                        return -5;
                    }

                    memset(f, 0xFF, sizeof(sylverant_frame_t));

                    cur_item = (sylverant_item_t *)f;
                    f->base.item_code = code;
                    f->base.auto_reject = 0;
                    f->base.reject_max = 0;
                    f->base.versions = 0;

                    if(handle_frame(n, f)) {
                        return -6;
                    }

                    break;
                }

                case ITEM_SUBTYPE_BARRIER:
                {
                    sylverant_barrier_t *b;

                    b = (sylverant_barrier_t *)
                    malloc(sizeof(sylverant_barrier_t));
                    if(!b) {
                        debug(DBG_ERROR, "Couldn't allocate space for item\n");
                        perror("malloc");
                        return -7;
                    }

                    memset(b, 0xFF, sizeof(sylverant_barrier_t));

                    cur_item = (sylverant_item_t *)b;
                    b->base.item_code = code;
                    b->base.auto_reject = 0;
                    b->base.reject_max = 0;
                    b->base.versions = 0;

                    if(handle_barrier(n, b)) {
                        return -8;
                    }

                    break;
                }

                case ITEM_SUBTYPE_UNIT:
                {
                    sylverant_unit_t *u;

                    u = (sylverant_unit_t *)malloc(sizeof(sylverant_unit_t));
                    if(!u) {
                        debug(DBG_ERROR, "Couldn't allocate space for item\n");
                        perror("malloc");
                        return -9;
                    }

                    memset(u, 0xFF, sizeof(sylverant_unit_t));

                    cur_item = (sylverant_item_t *)u;
                    u->base.item_code = code;
                    u->base.auto_reject = 0;
                    u->base.reject_max = 0;
                    u->base.versions = 0;

                    /* Since this can be -1 or -2, set it to an off-the-wall
                       value. */
                    u->max_plus = INT_MIN;
                    u->min_plus = INT_MIN;

                    if(handle_unit(n, u)) {
                        return -10;
                    }

                    break;
                }

                default:
                    /* Invalid subtype, bail */
                    debug(DBG_ERROR, "Invalid item subtype: %02x\n",
                          (code >> 8) & 0xFF);
                    return -11;
            }

            TAILQ_INSERT_TAIL(l->guards, cur_item, qentry);
            break;

        case ITEM_TYPE_MAG:
        {
            sylverant_mag_t *m;

            m = (sylverant_mag_t *)malloc(sizeof(sylverant_mag_t));
            if(!m) {
                debug(DBG_ERROR, "Couldn't allocate space for item\n");
                perror("malloc");
                return -12;
            }

            memset(m, 0xFF, sizeof(sylverant_mag_t));

            cur_item = (sylverant_item_t *)m;
            m->base.item_code = code;
            m->base.auto_reject = 0;
            m->base.reject_max = 0;
            m->base.versions = 0;
            m->allowed_colors = l->default_colors;
            m->allowed_cpb = l->default_cpb;
            m->allowed_rpb = l->default_rpb;
            m->allowed_lpb = l->default_lpb;
            TAILQ_INSERT_TAIL(l->mags, cur_item, qentry);

            if(handle_mag(n, m)) {
                return -13;
            }

            break;
        }

        case ITEM_TYPE_TOOL:
        {
            sylverant_tool_t *t;

            t = (sylverant_tool_t *)malloc(sizeof(sylverant_tool_t));
            if(!t) {
                debug(DBG_ERROR, "Couldn't allocate space for item\n");
                perror("malloc");
                return -14;
            }

            memset(t, 0xFF, sizeof(sylverant_tool_t));

            cur_item = (sylverant_item_t *)t;
            t->base.item_code = code;
            t->base.auto_reject = 0;
            t->base.reject_max = 0;
            t->base.versions = 0;
            TAILQ_INSERT_TAIL(l->tools, cur_item, qentry);

            if(handle_tool(n, t)) {
                return -15;
            }

            break;
        }

        default:
            /* Unknown item */
            debug(DBG_ERROR, "Unknown item type: %02x\n", code & 0xFF);
            return -16;
    }

    return 0;
}

int sylverant_read_limits(const char *f, sylverant_limits_t **l) {
    xmlParserCtxtPtr cxt;
    xmlDoc *doc;
    xmlNode *n;
    sylverant_limits_t *rv;
    int swap_code, irv = 0;
    xmlChar *bo, *def_act, *srank, *pbs;

    /* I'm lazy, and don't feel like typing this that many times. */
    typedef struct sylverant_item_queue iq_t;

    /* Make sure the file exists and can be read, otherwise quietly bail out */
    if(access(f, R_OK)) {
        return -1;
    }

    /* Allocate space for the base of the list. */
    rv = (sylverant_limits_t *)malloc(sizeof(sylverant_limits_t));

    if(!rv) {
        debug(DBG_ERROR, "Cannot make space for items list\n");
        *l = NULL;
        return -2;
    }

    /* Clear out the list. */
    memset(rv, 0, sizeof(sylverant_limits_t));

    /* Allocate space for each of the buckets. */
    rv->weapons = (iq_t *)malloc(sizeof(iq_t));
    rv->guards = (iq_t *)malloc(sizeof(iq_t));
    rv->mags = (iq_t *)malloc(sizeof(iq_t));
    rv->tools = (iq_t *)malloc(sizeof(iq_t));

    if(!rv->weapons || !rv->guards || !rv->mags || !rv->tools) {
        /* The standard says that free(NULL) does nothing, so... this works. */
        free(rv->weapons);
        free(rv->guards);
        free(rv->mags);
        free(rv->tools);
        free(rv);
        *l = NULL;
        return -3;
    }

    /* Initialize the buckets. After this block, its safe to actually use the
       sylverant_free_limits function to clean up things. */
    TAILQ_INIT(rv->weapons);
    TAILQ_INIT(rv->guards);
    TAILQ_INIT(rv->mags);
    TAILQ_INIT(rv->tools);

    /* Set the default behavior for photon blasts/colors */
    rv->default_cpb = rv->default_rpb = rv->default_lpb = 0xFF;
    rv->default_colors = 0xFFFF;

    /* Create an XML Parsing context */
    cxt = xmlNewParserCtxt();
    if(!cxt) {
        debug(DBG_ERROR, "Couldn't create parsing context for item list\n");
        irv = -4;
        goto err;
    }

    /* Open the configuration file for reading. */
    doc = xmlReadFile(f, NULL, XML_PARSE_DTDVALID);

    if(!doc) {
        xmlParserError(cxt, "Error in parsing item list");
        irv = -5;
        goto err_cxt;
    }

    /* Make sure the document validated properly. */
    if(!cxt->valid) {
        xmlParserValidityError(cxt, "Validity Error parsing item list");
        irv = -6;
        goto err_doc;
    }

    /* If we've gotten this far, we have a valid document, go through and read
       everything contained within... */
    n = xmlDocGetRootElement(doc);

    if(!n) {
        debug(DBG_ERROR, "Empty item list document\n");
        irv = -7;
        goto err_doc;
    }

    /* Make sure the config looks sane. */
    if(xmlStrcmp(n->name, XC"items")) {
        debug(DBG_ERROR, "Items list does not appear to be the right type\n");
        irv = -8;
        goto err_doc;
    }

    /* Grab the attributes of the root element */
    bo = xmlGetProp(n, XC"byteorder");
    def_act = xmlGetProp(n, XC"default");
    srank = xmlGetProp(n, XC"check_sranks");
    pbs = xmlGetProp(n, XC"check_pbs");

    if(!bo || !def_act || !srank || !pbs) {
        debug(DBG_ERROR, "items tag missing required attribute\n");
        irv = -9;
        xmlFree(bo);
        xmlFree(def_act);
        xmlFree(srank);
        xmlFree(pbs);
        
        goto err_doc;
    }

    /* Grab what we need from the attributes */
    if(!xmlStrcmp(bo, XC"big")) {
        swap_code = 1;
    }
    else if(!xmlStrcmp(bo, XC"little")) {
        swap_code = 0;
    }
    else {
        debug(DBG_WARN, "Unknown byte order, assuming little endian\n");
        swap_code = 0;
    }

    if(!xmlStrcmp(def_act, XC"reject")) {
        rv->default_behavior = ITEM_DEFAULT_REJECT;
    }
    else if(!xmlStrcmp(def_act, XC"allow")) {
        rv->default_behavior = ITEM_DEFAULT_ALLOW;
    }
    else {
        debug(DBG_WARN, "Unknown default behavior, assuming allow\n");
        rv->default_behavior = ITEM_DEFAULT_ALLOW;
    }

    if(!xmlStrcmp(srank, "true")) {
        rv->check_srank_names = 1;
    }
    else if(!xmlStrcmp(srank, "false")) {
        rv->check_srank_names = 0;
    }
    else {
        debug(DBG_WARN, "Unknown value for check_sranks, assuming true\n");
        rv->check_srank_names = 1;
    }

    if(!xmlStrcmp(pbs, "true")) {
        rv->check_pbs = 1;
    }
    else if(!xmlStrcmp(pbs, "false")) {
        rv->check_pbs = 0;
    }
    else {
        debug(DBG_WARN, "Unknown value for check_pbs, assuming true\n");
        rv->check_pbs = 1;
    }

    /* We're done with them, so clean them up... */
    xmlFree(bo);
    xmlFree(def_act);
    xmlFree(srank);
    xmlFree(pbs);    

    n = n->children;
    while(n) {
        if(n->type != XML_ELEMENT_NODE) {
            /* Ignore non-elements. */
            n = n->next;
            continue;
        }
        else if(!xmlStrcmp(n->name, XC"item")) {
            if(handle_item(n, rv, swap_code)) {
                irv = -10;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"pbs")) {
            if(handle_pbs(n, &rv->default_cpb, &rv->default_rpb,
                            &rv->default_lpb)) {
                irv = -11;
                goto err_doc;
            }
        }
        else if(!xmlStrcmp(n->name, XC"colors")) {
            if(handle_colors(n, &rv->default_colors)) {
                irv = -12;
                goto err_doc;
            }
        }
        else {
            debug(DBG_WARN, "Invalid Tag %s on line %hu\n", (char *)n->name,
                  n->line);
        }

        n = n->next;
    }

    /* If we get here, parsing finished fine... */
    irv = 0;
    *l = rv;

    /* Cleanup/error handling below... */
err_doc:
    if(irv < 0) {
        sylverant_free_limits(rv);
    }

    xmlFreeDoc(doc);
err_cxt:
    xmlFreeParserCtxt(cxt);
err:
    return 0;
}

static void clean_list(struct sylverant_item_queue *q) {
    sylverant_item_t *i, *tmp;

    /* Iterate through, freeing each entry. */
    i = TAILQ_FIRST(q);

    while(i) {
        tmp = TAILQ_NEXT(i, qentry);
        TAILQ_REMOVE(q, i, qentry);
        free(i);
        i = tmp;
    }

    /* Free the header of the list. */
    free(q);
}

int sylverant_free_limits(sylverant_limits_t *l) {
    sylverant_item_t *i, *tmp;

    /* Go through each list to clean up the information in it. */
    if(l->weapons) {
        clean_list(l->weapons);
        l->weapons = NULL;
    }

    if(l->guards) {
        clean_list(l->guards);
        l->guards = NULL;
    }

    if(l->mags) {
        clean_list(l->mags);
        l->mags = NULL;
    }

    if(l->tools) {
        clean_list(l->tools);
        l->tools = NULL;
    }

    /* Now that that is done, free the limits structure too. */
    free(l);

    return 0;
}

static int check_weapon(sylverant_limits_t *l, sylverant_iitem_t *i,
                        uint32_t version, uint32_t ic) {
    sylverant_item_t *j;
    sylverant_weapon_t *w;
    int is_srank = 0, is_named_srank = 0;
    uint8_t tmp;
    int is_special_weapon = i->data_b[4] == 0x80;

    /* Grab the real item type, if its a v2 item */
    if(i->data_b[5]) {
        ic = (i->data_b[5] << 8);
    }

    /* Figure out if we're looking at a S-Rank or not */
    if(ic >= 0x007000 && ic <= 0x008800) {
        is_srank = 1;

        /* If we're looking at a S-Rank, figure out if it has a name */
        if(i->data_b[6] >= 0x0C) {
            is_named_srank = 1;

            if(l->check_srank_names) {
                /* Make sure this S-Rank doesn't have the "name-only" flag set,
                   since that's not legitimately available in game. */
                if((i->data_b[6] & 0x88) == 0x88) {
                    return 0;
                }
                
                /* Check each character of the S-Rank name for validity. Only
                   A-Z are legitimately available. */
                /* First character */
                tmp = ((i->data_b[6] & 0x03) << 3) |
                    ((i->data_b[7] & 0xE0) >> 5);
                if(tmp > 26) {
                    return 0;
                }

                /* Second character */
                tmp = i->data_b[7] & 0x1F;
                if(tmp > 26) {
                    return 0;
                }

                /* Third character */
                tmp = ((i->data_b[8] >> 2) & 0x1F);
                if(tmp > 26) {
                    return 0;
                }

                /* Fourth character */
                tmp = ((i->data_b[8] & 0x03) << 3) |
                    ((i->data_b[9] & 0xE0) >> 5);
                if(tmp > 26) {
                    return 0;
                }

                /* Fifth character */
                tmp = i->data_b[9] & 0x1F;
                if(tmp > 26) {
                    return 0;
                }

                /* Sixth character */
                tmp = ((i->data_b[10] >> 2) & 0x1F);
                if(tmp > 26) {
                    return 0;
                }

                /* Seventh character */
                tmp = ((i->data_b[10] & 0x03) << 3) |
                    ((i->data_b[11] & 0xE0) >> 5);
                if(tmp > 26) {
                    return 0;
                }

                /* Eighth character */
                tmp = i->data_b[11] & 0x1F;
                if(tmp > 26) {
                    return 0;
                }
            }
        }
        else if(l->check_srank_names) {
            /* If we've set the flag to check S-Rank names, then if it doesn't
               have a name, it isn't legit. */
            return 0;
        }
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->weapons, qentry) {
        if(j->item_code == ic && (j->versions & version) == version) {
            w = (sylverant_weapon_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check the grind value first -- we have to ignore these on
               SPECIAL WEAPONs, since PSO is screwy in dealing with them... */
            if(((w->max_grind != -1 && i->data_b[3] > w->max_grind) ||
                (w->min_grind != -1 && i->data_b[3] < w->min_grind)) &&
               !is_special_weapon) {
                return 0;
            }

            /* Check each percent */
            if(!is_named_srank) {
                if(w->max_percents != -1) {
                    if((i->data_b[6] && (s8)i->data_b[7] > w->max_percents) ||
                       (i->data_b[8] && (s8)i->data_b[9] > w->max_percents) ||
                       (i->data_b[10] && (s8)i->data_b[11] > w->max_percents)) {
                        return 0;
                    }
                }

                if(w->min_percents != -1) {
                    if((i->data_b[6] && (s8)i->data_b[7] < w->min_percents) ||
                       (i->data_b[8] && (s8)i->data_b[9] < w->min_percents) ||
                       (i->data_b[10] && (s8)i->data_b[11] < w->min_percents)) {
                        return 0;
                    }
                }
            }

            /* Check if the attribute of the weapon is valid */
            tmp = i->data_b[4] & 0x7F;
            if(tmp > Weapon_Attr_MAX) {
                return 0;
            }

            if(!(w->valid_attrs & (1 << tmp))) {
                return 0;
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

static int check_guard(sylverant_limits_t *l, sylverant_iitem_t *i,
                       uint32_t version, uint32_t ic) {
    sylverant_item_t *j;
    sylverant_frame_t *f;
    sylverant_barrier_t *b;
    sylverant_unit_t *u;
    uint16_t dfp, evp;
    int16_t plus;
    int type = (ic >> 8) & 0x03;

    /* Grab the real item type, if its a v2 item */
    if(type != ITEM_SUBTYPE_UNIT && i->data_b[3]) {
        ic = ic | (i->data_b[3] << 16);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->guards, qentry) {
        if(j->item_code == ic && (j->versions & version) == version) {
            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check type specific things */
            switch(type) {
                case ITEM_SUBTYPE_FRAME:
                    f = (sylverant_frame_t *)j;

                    /* Check if the frame has too many slots */
                    if((f->max_slots != -1 && i->data_b[5] > f->max_slots) ||
                       (f->min_slots != -1 && i->data_b[5] < f->min_slots)) {
                        return 0;
                    }

                    /* Check if the dfp boost is too high */
                    dfp = i->data_b[6] | (i->data_b[7] << 8);
                    if((f->max_dfp != -1 && dfp > f->max_dfp) ||
                       (f->min_dfp != -1 && dfp < f->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    evp = i->data_b[8] | (i->data_b[9] << 8);
                    if((f->max_evp != -1 && evp > f->max_evp) ||
                       (f->min_evp != -1 && evp < f->min_evp)) {
                        return 0;
                    }

                    /* See if its maxed and we're supposed to reject that */
                    if(f->base.reject_max && dfp == f->max_dfp &&
                       evp == f->max_evp) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_BARRIER:
                    b = (sylverant_barrier_t *)j;

                    /* Check if the dfp boost is too high */
                    dfp = i->data_b[6] | (i->data_b[7] << 8);
                    if((b->max_dfp != -1 && dfp > b->max_dfp) ||
                       (b->min_dfp != -1 && dfp < b->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    evp = i->data_b[8] | (i->data_b[9] << 8);
                    if((b->max_evp != -1 && evp > b->max_evp) ||
                       (b->min_evp != -1 && evp < b->min_evp)) {
                        return 0;
                    }

                    /* See if its maxed and we're supposed to reject that */
                    if(b->base.reject_max && dfp == b->max_dfp &&
                       evp == b->max_evp) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_UNIT:
                    u = (sylverant_unit_t *)j;

                    /* Check the Plus/Minus number */
                    plus = i->data_b[6] | (i->data_b[7] << 8);
                    if((u->max_plus != INT_MIN && plus > u->max_plus) ||
                       (u->min_plus != INT_MIN && plus < u->min_plus)) {
                        return 0;
                    }

                    break;
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

static int check_mag(sylverant_limits_t *l, sylverant_iitem_t *i,
                     uint32_t version, uint32_t ic) {
    sylverant_item_t *j;
    sylverant_mag_t *m;
    uint16_t tmp;
    int level = 0;
    int cpb, rpb, lpb, hascpb, hasrpb, haslpb;

    /* Grab the real item type, if its a v2 item, otherwise chop down to only
       16-bits */
    if(i->data_b[1] == 0x00 && i->data_b[2] >= 0xC9) {
        ic = 0x02 | (((i->data_b[2] - 0xC9) + 0x2C) << 8);
    }
    else {
        ic = 0x02 | (i->data_b[1] << 8);
    }

    /* Grab the photon blasts */
    cpb = i->data_b[3] & 0x07;
    rpb = (i->data_b[3] >> 3) & 0x07;
    lpb = (i->data_b[3] >> 6) & 0x03;

    /* Figure out what slots should have PBs */
    hascpb = i->data2_b[3] & 0x80;
    hasrpb = i->data_b[5] & 0x80;
    haslpb = i->data_b[7] & 0x80;

    /* If we're supposed to check for obviously hacked PBs, do so */
    if(l->check_pbs) {
        /* Disallow hacked photon blasts (that likely crash v1 clients) */
        if(cpb > 5 || rpb > 5) {
            return 0;
        }

        /* Make sure that they're marked to have PBs if they have one */
        if(cpb && !hascpb) {
            return 0;
        }

        if(rpb && !hasrpb) {
            return 0;
        }

        if(lpb && !haslpb) {
            return 0;
        }

        /* Make sure there's no overlap between center and right (left can't
           overlap at all by design) */
        if(hascpb && hasrpb && cpb == rpb) {
            return 0;
        }
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->mags, qentry) {
        if(j->item_code == ic && (j->versions & version) == version) {
            m = (sylverant_mag_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check the mag's DEF */
            tmp = (i->data_b[4] | (i->data_b[5] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_def != -1 && tmp > m->max_def) ||
               (m->min_def != -1 && tmp < m->min_def)) {
                return 0;
            }

            /* Check the mag's POW */
            tmp = (i->data_b[6] | (i->data_b[7] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_pow != -1 && tmp > m->max_pow) ||
               (m->min_pow != -1 && tmp < m->min_pow)) {
                return 0;
            }

            /* Check the mag's DEX */
            tmp = (i->data_b[8] | (i->data_b[9] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_dex != -1 && tmp > m->max_dex) ||
               (m->min_dex != -1 && tmp < m->min_dex)) {
                return 0;
            }

            /* Check the mag's MIND */
            tmp = (i->data_b[10] | (i->data_b[11] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_mind != -1 && tmp > m->max_mind) ||
               (m->min_mind != -1 && tmp < m->min_mind)) {
                return 0;
            }

            /* Check the level */
            if((m->max_level != -1 && level > m->max_level) ||
               (m->min_level != -1 && level < m->min_level)) {
                return 0;
            }

            /* Check the IQ */
            tmp = i->data2_b[0] | (i->data2_b[1] << 8);
            if((m->max_iq != -1 && tmp > m->max_iq) ||
               (m->min_iq != -1 && tmp < m->min_iq)) {
                return 0;
            }

            /* Check the synchro */
            tmp = i->data2_b[2] | (i->data2_b[3] << 8) & 0x7FFF;
            if((m->max_synchro != -1 && tmp > m->max_synchro) ||
               (m->min_synchro != -1 && tmp < m->min_synchro)) {
                return 0;
            }

            /* Figure out what the real left PB is... This is kinda ugly... */
            if(haslpb) {
                if(cpb <= lpb && rpb <= lpb) {
                    lpb += 2;
                }
                else if(cpb <= lpb) {
                    ++lpb;

                    if(rpb == lpb) {
                        ++lpb;
                    }
                }
                else if(rpb <= lpb) {
                    ++lpb;

                    if(cpb == lpb) {
                        ++lpb;
                    }
                }
            }

            /* Now, actually make sure that things are happy with the pbs that
               are actually selected */
            if(hascpb && !(m->allowed_cpb & (1 << cpb))) {
                return 0;
            }

            if(hasrpb && !(m->allowed_rpb & (1 << rpb))) {
                return 0;
            }

            if(haslpb && !(m->allowed_lpb & (1 << lpb))) {
                return 0;
            }

            /* Parse out what the color is and check it */
            tmp = (i->data_b[4] & 0x01) | ((i->data_b[6] & 0x01) << 1) |
                ((i->data_b[8] & 0x01) << 2) | ((i->data_b[10] & 0x01) << 3);

            if(!(m->allowed_colors & (1 << tmp))) {
                return 0;
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

static int check_tool(sylverant_limits_t *l, sylverant_iitem_t *i,
                      uint32_t version, uint32_t ic) {
    sylverant_item_t *j;
    sylverant_tool_t *t;

    /* Grab the real item type, if its a v2 item */
    if(ic == 0x060D03 && i->data_b[3]) {
        ic = 0x000E03 | ((i->data_b[3] - 1) << 16);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->tools, qentry) {
        if(j->item_code == ic && (j->versions & version) == version) {
            t = (sylverant_tool_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check if the user has too many of this tool */
            if((t->max_stack != -1 && i->data_b[5] > t->max_stack) ||
               (t->min_stack != -1 && i->data_b[5] < t->min_stack)) {
                return 0;
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

int sylverant_limits_check_item(sylverant_limits_t *l, sylverant_iitem_t *i,
                                uint32_t version) {
    uint32_t item_code = i->data_b[0] | (i->data_b[1] << 8) |
        (i->data_b[2] << 16);

    switch(item_code & 0xFF) {
        case ITEM_TYPE_WEAPON:
            return check_weapon(l, i, version, item_code);

        case ITEM_TYPE_GUARD:
            return check_guard(l, i, version, item_code);

        case ITEM_TYPE_MAG:
            return check_mag(l, i, version, item_code);

        case ITEM_TYPE_TOOL:
            return check_tool(l, i, version, item_code);

        case ITEM_TYPE_MESETA:
            /* Always pass... */
            return 1;
    }

    /* Reject unknown item types... they'll probably crash people anyway. */
    return 0;
}

const char *sylverant_weapon_attr_name(sylverant_weapon_attr_t num) {
    if(num > Weapon_Attr_MAX) {
        return NULL;
    }

    return weapon_attrs[num];
}
