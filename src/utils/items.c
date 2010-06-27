/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2010 Lawrence Sebald

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
#include <expat.h>
#include <errno.h>

#include "sylverant/items.h"

#define BUF_SIZE 8192
#define DIE() XML_StopParser(parser, 0); return

static XML_Parser parser = NULL;
static int in_items = 0, swap_code = 0;
static sylverant_item_t *cur_item = NULL;
static void (*cur_hnd)(const XML_Char *name, const XML_Char **attrs);

static void handle_items(sylverant_limits_t *l, const XML_Char **attrs) {
    int i;

    if(in_items) {
        /* Not allowed to have <items> inside of <items>. */
        DIE();
    }

    in_items = 1;

    for(i = 0; attrs[i]; i += 2) {
        if(!strcmp(attrs[i], "byteorder")) {
            if(!strcmp(attrs[i + 1], "little")) {
                swap_code = 0;
            }
            else if(!strcmp(attrs[i + 1], "big")) {
                swap_code = 1;
            }
            else {
                /* Invalid option, bail. */
                DIE();
            }
        }
        else if(!strcmp(attrs[i], "default")) {
            if(!strcmp(attrs[i + 1], "allow")) {
                l->default_behavior = ITEM_DEFAULT_ALLOW;
            }
            else if(!strcmp(attrs[i + 1], "reject")) {
                l->default_behavior = ITEM_DEFAULT_REJECT;
            }
            else {
                /* Invalid option, bail. */
                DIE();
            }
        }
        else {
            /* Unknown attribute, bail. */
            DIE();
        }
    }
}

static int common_tag(const XML_Char *name, const XML_Char **attrs) {
    int i;

    if(!strcmp(name, "versions")) {
        cur_item->allowed_versions = 0;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "v1")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->allowed_versions |= ITEM_VERSION_V1;
                }
                else if(strcmp(attrs[i + 1], "false")) {
                    /* If its not true, and not false, bail */
                    DIE();
                }
            }
            else if(!strcmp(attrs[i], "v2")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->allowed_versions |= ITEM_VERSION_V2;
                }
                else if(strcmp(attrs[i + 1], "false")) {
                    /* If its not true, and not false, bail */
                    DIE();
                }
            }
            else if(!strcmp(attrs[i], "gc")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->allowed_versions |= ITEM_VERSION_GC;
                }
                else if(strcmp(attrs[i + 1], "false")) {
                    /* If its not true, and not false, bail */
                    DIE();
                }
            }
            else {
                DIE();
            }
        }
    }
    else if(!strcmp(name, "auto_reject")) {
        /* No attributes on the <auto_reject> tag. */
        if(attrs && attrs[0]) {
            DIE();
        }

        cur_item->auto_reject = 1;
    }
    else {
        return 0;
    }

    return 1;
}

static int parse_max(const XML_Char **attrs) {
    int rv;

    if(!attrs || !attrs[0] || !attrs[1] || strcmp(attrs[0], "max") ||
       attrs[2]) {
        XML_StopParser(parser, 0);
        return -1;
    }

    errno = 0;
    rv = (int)strtol(attrs[1], NULL, 0);
    if(errno) {
        XML_StopParser(parser, 0);
        return -1;
    }

    return rv;
}

static void handle_weapon(const XML_Char *name, const XML_Char **attrs) {
    sylverant_weapon_t *w = (sylverant_weapon_t *)cur_item;

    if(!strcmp(name, "grind")) {
        w->max_grind = parse_max(attrs);
    }
    else if(!strcmp(name, "percents")) {
        w->max_percents = parse_max(attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_frame(const XML_Char *name, const XML_Char **attrs) {
    sylverant_frame_t *f = (sylverant_frame_t *)cur_item;

    if(!strcmp(name, "slots")) {
        f->max_slots = parse_max(attrs);
    }
    else if(!strcmp(name, "dfp")) {
        f->max_dfp = parse_max(attrs);
    }
    else if(!strcmp(name, "evp")) {
        f->max_evp = parse_max(attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_barrier(const XML_Char *name, const XML_Char **attrs) {
    sylverant_barrier_t *b = (sylverant_barrier_t *)cur_item;

    if(!strcmp(name, "dfp")) {
        b->max_dfp = parse_max(attrs);
    }
    else if(!strcmp(name, "evp")) {
        b->max_evp = parse_max(attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_unit(const XML_Char *name, const XML_Char **attrs) {
    /* Units only support common tags for now... */
    if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_mag(const XML_Char *name, const XML_Char **attrs) {
    sylverant_mag_t *m = (sylverant_mag_t *)m;

    if(!strcmp(name, "level")) {
        m->max_level = parse_max(attrs);
    }
    else if(!strcmp(name, "def")) {
        m->max_def = parse_max(attrs);
    }
    else if(!strcmp(name, "pow")) {
        m->max_pow = parse_max(attrs);
    }
    else if(!strcmp(name, "dex")) {
        m->max_dex = parse_max(attrs);
    }
    else if(!strcmp(name, "mind")) {
        m->max_mind = parse_max(attrs);
    }
    else if(!strcmp(name, "synchro")) {
        m->max_synchro = parse_max(attrs);
    }
    else if(!strcmp(name, "iq")) {
        m->max_iq = parse_max(attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_tool(const XML_Char *name, const XML_Char **attrs) {
    sylverant_tool_t *t = (sylverant_tool_t *)cur_item;

    if(!strcmp(name, "stack")) {
        t->max_stack = parse_max(attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_item(sylverant_limits_t *l, const XML_Char **attrs) {
    int i;
    uint32_t code;

    /* The only attribute on <item> is the code one. */
    if(!attrs || !attrs[0] || !attrs[1] || strcmp(attrs[0], "code")) {
        DIE();
    }

    /* Grab the hex code. */
    errno = 0;
    code = (uint32_t)strtoul(attrs[1], NULL, 16);
    if(errno) {
        DIE();
    }

   if(swap_code) {
       code = ((code >> 24) & 0xFF) | ((code >> 8) & 0xFF00) |
           ((code & 0xFF00) << 8) | ((code & 0xFF) << 24);
   }

    /* Now that we have the item code, we can figure out what type of item we
       actually are allocating. */
    switch(code & 0xFF) {
        case ITEM_TYPE_WEAPON:
        {
            sylverant_weapon_t *w;

            w = (sylverant_weapon_t *)malloc(sizeof(sylverant_weapon_t));
            if(!w) {
                DIE();
            }

            cur_item = (sylverant_item_t *)w;
            cur_hnd = &handle_weapon;
            w->base.item_code = code;
            TAILQ_INSERT_TAIL(l->weapons, cur_item, qentry);
            break;
        }

        case ITEM_TYPE_GUARD:
            switch((code >> 8) & 0xFF) {
                case ITEM_SUBTYPE_FRAME:
                {
                    sylverant_frame_t *f;

                    f = (sylverant_frame_t *)malloc(sizeof(sylverant_frame_t));
                    if(!f) {
                        DIE();
                    }

                    cur_item = (sylverant_item_t *)f;
                    cur_hnd = &handle_frame;
                    f->base.item_code = code;
                    break;
                }

                case ITEM_SUBTYPE_BARRIER:
                {
                    sylverant_barrier_t *b;

                    b = (sylverant_barrier_t *)
                        malloc(sizeof(sylverant_barrier_t));
                    if(!b) {
                        DIE();
                    }

                    cur_item = (sylverant_item_t *)b;
                    cur_hnd = &handle_barrier;
                    b->base.item_code = code;
                    break;
                }

                case ITEM_SUBTYPE_UNIT:
                {
                    sylverant_unit_t *u;

                    u = (sylverant_unit_t *)malloc(sizeof(sylverant_unit_t));
                    if(!u) {
                        DIE();
                    }

                    cur_item = (sylverant_item_t *)u;
                    cur_hnd = &handle_unit;
                    u->base.item_code = code;
                    break;
                }

                default:
                    /* Invalid subtype, bail */
                    DIE();
            }

            TAILQ_INSERT_TAIL(l->guards, cur_item, qentry);
            break;

        case ITEM_TYPE_MAG:
        {
            sylverant_mag_t *m;

            m = (sylverant_mag_t *)malloc(sizeof(sylverant_mag_t));
            if(!m) {
                DIE();
            }

            cur_item = (sylverant_item_t *)m;
            cur_hnd = &handle_mag;
            m->base.item_code = code;
            TAILQ_INSERT_TAIL(l->mags, cur_item, qentry);
            break;
        }

        case ITEM_TYPE_TOOL:
        {
            sylverant_tool_t *t;

            t = (sylverant_tool_t *)malloc(sizeof(sylverant_tool_t));
            if(!t) {
                DIE();
            }

            cur_item = (sylverant_item_t *)t;
            cur_hnd = &handle_tool;
            t->base.item_code = code;
            TAILQ_INSERT_TAIL(l->tools, cur_item, qentry);
            break;
        }

        default:
            /* Unknown item */
            DIE();
    }

   /* Default to all versions OK */
   cur_item->allowed_versions = ITEM_VERSION_ALL;
}

static void item_start_hnd(void *d, const XML_Char *name,
                           const XML_Char **attrs) {
    sylverant_limits_t *l = *(sylverant_limits_t **)d;

    if(!strcmp(name, "items")) {
        handle_items(l, attrs);
    }
    else if(!strcmp(name, "item") && in_items) {
        handle_item(l, attrs);
    }
    else if(cur_item && in_items && cur_hnd) {
        cur_hnd(name, attrs);
    }
    else {
        DIE();
    }
}

static void item_end_hnd(void *d, const XML_Char *name) {
    if(!strcmp(name, "items")) {
        in_items = 0;
    }
    else if(!strcmp(name, "item")) {
        cur_item = NULL;
        cur_hnd = NULL;
    }
}

int sylverant_read_limits(const char *f, sylverant_limits_t **l) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;
    sylverant_limits_t *rv;

    /* I'm lazy, and don't feel like typing this that many times. */
    typedef struct sylverant_item_queue iq_t;

    /* Only one parser allowed at a time. */
    if(parser) {
        return -6;
    }

    /* Allocate space for the base of the list. */
    rv = (sylverant_limits_t *)malloc(sizeof(sylverant_limits_t));

    if(!rv) {
        *l = NULL;
        return -4;
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
        return -5;
    }

    /* Initialize the buckets. After this block, its safe to actually use the
       sylverant_clean_limits function to clean up things. */
    TAILQ_INIT(rv->weapons);
    TAILQ_INIT(rv->guards);
    TAILQ_INIT(rv->mags);
    TAILQ_INIT(rv->tools);

    /* Set the default limits behavior to allow unknown items. */
    rv->default_behavior = ITEM_DEFAULT_ALLOW;

    /* Open the configuration file for reading. */
    fp = fopen(f, "r");

    if(!fp) {
        sylverant_clean_limits(rv);
        *l = NULL;
        return -1;
    }

    /* Create the XML parser object. */
    p = XML_ParserCreate(NULL);

    if(!p)  {
        fclose(fp);
        sylverant_clean_limits(rv);
        *l = NULL;
        return -2;
    }

    XML_SetElementHandler(p, &item_start_hnd, &item_end_hnd);

    /* Set up the user data so we can store the configuration. */
    XML_SetUserData(p, &rv);

    parser = p;

    for(;;) {
        /* Grab the buffer to read into. */
        buf = XML_GetBuffer(p, BUF_SIZE);

        if(!buf)    {
            XML_ParserFree(p);
            sylverant_clean_limits(rv);
            fclose(fp);
            *l = NULL;
            parser = NULL;
            in_items = swap_code = 0;
            cur_item = NULL;
            cur_hnd = NULL;
            return -2;
        }

        /* Read in from the file. */
        bytes = fread(buf, 1, BUF_SIZE, fp);

        if(bytes < 0)   {
            XML_ParserFree(p);
            sylverant_clean_limits(rv);
            fclose(fp);
            *l = NULL;
            parser = NULL;
            in_items = swap_code = 0;
            cur_item = NULL;
            cur_hnd = NULL;
            return -2;
        }

        /* Parse the bit we read in. */
        if(!XML_ParseBuffer(p, bytes, !bytes))  {
            XML_ParserFree(p);
            sylverant_clean_limits(rv);
            fclose(fp);
            *l = NULL;
            parser = NULL;
            in_items = swap_code = 0;
            cur_item = NULL;
            cur_hnd = NULL;
            return -3;
        }

        if(!bytes)  {
            break;
        }
    }

    XML_ParserFree(p);
    *l = rv;
    parser = NULL;
    in_items = swap_code = 0;
    cur_item = NULL;
    cur_hnd = NULL;

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

int sylverant_clean_limits(sylverant_limits_t *l) {
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
