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
#define DIE(x) XML_StopParser(parser, 0); return x

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

static int parse_max(const XML_Char **attrs, int *max, int *min) {
    int first = 0;

    /* Make sure the tag looks sane */
    if(!attrs || !attrs[0] || !attrs[1] || (attrs[2] && !attrs[3])) {
        DIE(-1);
    }

    /* Grab the first of the two possible attributes */
    errno = 0;

    if(!strcmp(attrs[0], "max")) {
        *max = (int)strtol(attrs[1], NULL, 0);
    }
    else if(!strcmp(attrs[0], "min")) {
        *min = (int)strtol(attrs[1], NULL, 0);
        first = 1;
    }
    else {
        DIE(-1);
    }

    /* If we have a second attribute, grab it */
    if(attrs[2]) {
        if(!strcmp(attrs[2], "max") && first) {
            *max = (int)strtol(attrs[3], NULL, 0);
        }
        else if(!strcmp(attrs[2], "min") && !first) {
            *min = (int)strtol(attrs[3], NULL, 0);
        }
        else {
            DIE(-1);
        }
    }

    /* If there was an error parsing the numbers out, die */
    if(errno) {
        DIE(-1);
    }

    return 0;
}

static void handle_weapon(const XML_Char *name, const XML_Char **attrs) {
    sylverant_weapon_t *w = (sylverant_weapon_t *)cur_item;

    if(!strcmp(name, "grind")) {
        if(parse_max(attrs, &w->max_grind, &w->min_grind)) {
            DIE();
        }
    }
    else if(!strcmp(name, "percents")) {
        if(parse_max(attrs, &w->max_percents, &w->min_percents)) {
            DIE();
        }
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_frame(const XML_Char *name, const XML_Char **attrs) {
    sylverant_frame_t *f = (sylverant_frame_t *)cur_item;

    if(!strcmp(name, "slots")) {
        if(parse_max(attrs, &f->max_slots, &f->min_slots)) {
            DIE();
        }
    }
    else if(!strcmp(name, "dfp")) {
        if(parse_max(attrs, &f->max_dfp, &f->min_dfp)) {
            DIE();
        }
    }
    else if(!strcmp(name, "evp")) {
        if(parse_max(attrs, &f->max_evp, &f->min_evp)) {
            DIE();
        }
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_barrier(const XML_Char *name, const XML_Char **attrs) {
    sylverant_barrier_t *b = (sylverant_barrier_t *)cur_item;

    if(!strcmp(name, "dfp")) {
        if(parse_max(attrs, &b->max_dfp, &b->min_dfp)) {
            DIE();
        }
    }
    else if(!strcmp(name, "evp")) {
        if(parse_max(attrs, &b->max_evp, &b->min_evp)) {
            DIE();
        }
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
    sylverant_mag_t *m = (sylverant_mag_t *)cur_item;

    if(!strcmp(name, "level")) {
        if(parse_max(attrs, &m->max_level, &m->min_level)) {
            DIE();
        }
    }
    else if(!strcmp(name, "def")) {
        if(parse_max(attrs, &m->max_def, &m->min_def)) {
            DIE();
        }
    }
    else if(!strcmp(name, "pow")) {
        if(parse_max(attrs, &m->max_pow, &m->min_pow)) {
            DIE();
        }
    }
    else if(!strcmp(name, "dex")) {
        if(parse_max(attrs, &m->max_dex, &m->min_dex)) {
            DIE();
        }
    }
    else if(!strcmp(name, "mind")) {
        if(parse_max(attrs, &m->max_mind, &m->min_mind)) {
            DIE();
        }
    }
    else if(!strcmp(name, "synchro")) {
        if(parse_max(attrs, &m->max_synchro, &m->min_synchro)) {
            DIE();
        }
    }
    else if(!strcmp(name, "iq")) {
        if(parse_max(attrs, &m->max_iq, &m->min_iq)) {
            DIE();
        }
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_tool(const XML_Char *name, const XML_Char **attrs) {
    sylverant_tool_t *t = (sylverant_tool_t *)cur_item;

    if(!strcmp(name, "stack")) {
        if(parse_max(attrs, &t->max_stack, &t->min_stack)) {
            DIE();
        }
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

            memset(w, 0, sizeof(sylverant_weapon_t));

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

                    memset(f, 0, sizeof(sylverant_frame_t));

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

                    memset(b, 0, sizeof(sylverant_barrier_t));

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

                    memset(u, 0, sizeof(sylverant_unit_t));

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

            memset(m, 0, sizeof(sylverant_mag_t));

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

            memset(t, 0, sizeof(sylverant_tool_t));

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

static int check_weapon(sylverant_limits_t *l, sylverant_iitem_t *i,
                        uint32_t ic) {
    sylverant_item_t *j;
    sylverant_weapon_t *w;

    /* Grab the real item type, if its a v2 item */
    if(i->data_b[5]) {
        ic = (i->data_b[5] << 8);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->weapons, qentry) {
        if(j->item_code == ic) {
            w = (sylverant_weapon_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check the grind value first */
            if((w->max_grind && i->data_b[3] > w->max_grind) ||
               (w->min_grind && i->data_b[3] < w->min_grind)) {
                return 0;
            }

            /* Check each percent */
            if(w->max_percents) {
                if(i->data_b[7] > w->max_percents ||
                   i->data_b[9] > w->max_percents ||
                   i->data_b[11] > w->max_percents) {
                    return 0;
                }
            }

            if(w->min_percents) {
                if(i->data_b[7] < w->min_percents ||
                   i->data_b[9] < w->min_percents ||
                   i->data_b[11] < w->min_percents) {
                    return 0;
                }
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

static int check_guard(sylverant_limits_t *l, sylverant_iitem_t *i,
                       uint32_t ic) {
    sylverant_item_t *j;
    sylverant_frame_t *f;
    sylverant_barrier_t *b;
    sylverant_unit_t *u;
    uint16_t tmp;
    int type = (ic >> 8) & 0x03;

    /* Grab the real item type, if its a v2 item */
    if(type != ITEM_SUBTYPE_UNIT && i->data_b[3]) {
        ic = ic | (i->data_b[3] << 16);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->guards, qentry) {
        if(j->item_code == ic) {
            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check type specific things */
            switch(type) {
                case ITEM_SUBTYPE_FRAME:
                    f = (sylverant_frame_t *)j;

                    /* Check if the frame has too many slots */
                    if((f->max_slots && i->data_b[5] > f->max_slots) ||
                       (f->min_slots && i->data_b[5] < f->min_slots)) {
                        return 0;
                    }
                        

                    /* Check if the dfp boost is too high */
                    tmp = i->data_b[6] | (i->data_b[7] << 8);
                    if((f->max_dfp && tmp > f->max_dfp) ||
                       (f->min_dfp && tmp < f->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    tmp = i->data_b[9] | (i->data_b[10] << 8);
                    if((f->max_evp && tmp > f->max_evp) ||
                       (f->min_evp && tmp < f->min_evp)) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_BARRIER:
                    b = (sylverant_barrier_t *)j;

                    /* Check if the dfp boost is too high */
                    tmp = i->data_b[6] | (i->data_b[7] << 8);
                    if((b->max_dfp && tmp > b->max_dfp) ||
                       (b->min_dfp && tmp < b->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    tmp = i->data_b[9] | (i->data_b[10] << 8);
                    if((b->max_evp && tmp > b->max_evp) ||
                       (b->min_evp && tmp < b->min_evp)) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_UNIT:
                    u = (sylverant_unit_t *)j;
                    /* Nothing to check... */
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
                     uint32_t ic) {
    sylverant_item_t *j;
    sylverant_mag_t *m;
    uint16_t tmp;
    int level = 0;

    /* Grab the real item type, if its a v2 item, otherwise chop down to only
       16-bits */
    if(i->data_b[1] == 0x00 && i->data_b[2] >= 0xC9) {
        ic = 0x02 | (((i->data_b[2] - 0xC9) + 0x2C) << 8);
    }
    else {
        ic = 0x02 | (i->data_b[1] << 8);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->mags, qentry) {
        if(j->item_code == ic) {
            m = (sylverant_mag_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check the mag's DEF */
            tmp = (i->data_b[4] | (i->data_b[5] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_def && tmp > m->max_def) ||
               (m->min_def && tmp < m->min_def)) {
                return 0;
            }

            /* Check the mag's POW */
            tmp = (i->data_b[6] | (i->data_b[7] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_pow && tmp > m->max_pow) ||
               (m->min_pow && tmp < m->min_pow)) {
                return 0;
            }

            /* Check the mag's DEX */
            tmp = (i->data_b[8] | (i->data_b[9] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_dex && tmp > m->max_dex) ||
               (m->min_dex && tmp < m->min_dex)) {
                return 0;
            }

            /* Check the mag's MIND */
            tmp = (i->data_b[10] | (i->data_b[11] << 8)) & 0x7FFE;
            tmp /= 100;
            level += tmp;

            if((m->max_mind && tmp > m->max_mind) ||
               (m->min_mind && tmp < m->min_mind)) {
                return 0;
            }

            /* Check the level */
            if((m->max_level && level > m->max_level) ||
               (m->min_level && level < m->min_level)) {
                return 0;
            }

            /* Check the IQ */
            tmp = i->data2_b[0] | (i->data2_b[1] << 8);
            if((m->max_iq && tmp > m->max_iq) ||
               (m->min_iq && tmp < m->min_iq)) {
                return 0;
            }

            /* Check the synchro */
            tmp = i->data2_b[2] | (i->data2_b[3] << 8) & 0x7FFF;
            if((m->max_synchro && tmp > m->max_synchro) ||
               (m->min_synchro && tmp < m->min_synchro)) {
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
                      uint32_t ic) {
    sylverant_item_t *j;
    sylverant_tool_t *t;

    /* Grab the real item type, if its a v2 item */
    if(ic == 0x060D03 && i->data_b[3]) {
        ic = 0x000E03 | ((i->data_b[3] - 1) << 16);
    }

    /* Find the item in our list, if its there */
    TAILQ_FOREACH(j, l->tools, qentry) {
        if(j->item_code == ic) {
            t = (sylverant_tool_t *)j;

            /* Autoreject if we're supposed to */
            if(j->auto_reject) {
                return 0;
            }

            /* Check if the user has too many of this tool */
            if((t->max_stack && i->data_b[5] > t->max_stack) ||
               (t->min_stack && i->data_b[5] < t->min_stack)) {
                return 0;
            }

            /* If we haven't rejected yet, accept */
            return 1;
        }
    }

    /* If we don't find it, do whatever the default is */
    return l->default_behavior;
}

int sylverant_limits_check_item(sylverant_limits_t *l, sylverant_iitem_t *i) {
    uint32_t item_code = i->data_b[0] | (i->data_b[1] << 8) |
        (i->data_b[2] << 16);

    switch(item_code & 0xFF) {
        case ITEM_TYPE_WEAPON:
            return check_weapon(l, i, item_code);

        case ITEM_TYPE_GUARD:
            return check_guard(l, i, item_code);;

        case ITEM_TYPE_MAG:
            return check_mag(l, i, item_code);

        case ITEM_TYPE_TOOL:
            return check_tool(l, i, item_code);
    }

    /* Reject unknown item types... they'll probably crash people anyway. */
    return 0;
}
