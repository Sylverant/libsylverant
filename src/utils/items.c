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
#include <limits.h>

#include "sylverant/items.h"

#define BUF_SIZE 8192
#define DIE() XML_StopParser(parser, 0); return

static XML_Parser parser = NULL;
static int in_items = 0, swap_code = 0;
static sylverant_item_t *cur_item = NULL;
static void (*cur_hnd)(const XML_Char *name, const XML_Char **attrs);

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
    "Farlla", "Estlla", "Leilla", "Pilla", "Golla", "Mylla+Youlla", /* Legit */
    "bad1", "bad2" /* Not legit */
};

#define NUM_COLORS 16

/* List of Mag colors */
static const char *mag_colors[NUM_COLORS] = {
    "red", "blue", "yellow", "green", "purple", "darkpurple", "white", "cyan",
    "brown", "black", "c10", "c11", "c12", "c13", "c14", "c15"
};

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
        else if(!strcmp(attrs[i], "check_sranks")) {
            if(!strcmp(attrs[i + 1], "true")) {
                l->check_srank_names = 1;
            }
            else if(!strcmp(attrs[i + 1], "false")) {
                l->check_srank_names = 0;
            }
            else {
                /* Invalid option, bail. */
                DIE();
            }
        }
        else if(!strcmp(attrs[i], "check_pbs")) {
            if(!strcmp(attrs[i + 1], "true")) {
                l->check_pbs = 1;
            }
            else if(!strcmp(attrs[i + 1], "false")) {
                l->check_pbs = 0;
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
        cur_item->versions = 0;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "v1")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->versions |= ITEM_VERSION_V1;
                }
                else if(strcmp(attrs[i + 1], "false")) {
                    /* If its not true, and not false, bail */
                    DIE();
                }
            }
            else if(!strcmp(attrs[i], "v2")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->versions |= ITEM_VERSION_V2;
                }
                else if(strcmp(attrs[i + 1], "false")) {
                    /* If its not true, and not false, bail */
                    DIE();
                }
            }
            else if(!strcmp(attrs[i], "gc")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cur_item->versions |= ITEM_VERSION_GC;
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

static void parse_max(const XML_Char **attrs, int *max, int *min) {
    int first = 0;

    /* Make sure the tag looks sane */
    if(!attrs || !attrs[0] || !attrs[1] || (attrs[2] && !attrs[3])) {
        DIE();
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
        DIE();
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
            DIE();
        }
    }

    /* If there was an error parsing the numbers out, die */
    if(errno) {
        DIE();
    }
}

static void parse_attrs(const XML_Char **attrs, uint64_t *valid) {
    int i;
    char *str, *lasts, *tok;

    /* Make sure we have a sane set */
    if(!attrs || !attrs[0] || !attrs[1] || attrs[2]) {
        DIE();
    }

    /* The only valid attribute here is disallow */
    if(strcmp("disallow", attrs[0])) {
        DIE();
    }

    /* Create a temporary copy of the string for parsing */
    str = strdup(attrs[1]);
    tok = strtok_r(str, ", ", &lasts);

    /* Go through any attributes in the disallow list */
    while(tok) {
        /* Look through the list of weapon attributes for what we have */
        for(i = 0; i <= Weapon_Attr_MAX; ++i) {
            if(!strcmp(weapon_attrs[i], tok)) {
                *valid &= ~(1 << i);
                break;
            }
        }

        /* If we didn't find the attribute, die */
        if(i > Weapon_Attr_MAX) {
            free(str);
            DIE();
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

    /* Clean up the temporary string, since we're done */
    free(str);
}

static void parse_pbs(const XML_Char **attrs, uint8_t *c, uint8_t *r,
                      uint8_t *l) {
    int i;
    char *str, *lasts, *tok;
    uint8_t *valid;

    /* Make sure we have a sane set */
    if(!attrs || !attrs[0] || !attrs[1] || !attrs[2] || !attrs[3] || attrs[4]) {
        DIE();
    }

    /* The first attribute must be the place we're setting */
    if(strcmp("pos", attrs[0])) {
        DIE();
    }

    if(!strcmp("center", attrs[1])) {
        valid = c;
    }
    else if(!strcmp("right", attrs[1])) {
        valid = r;
    }
    else if(!strcmp("left", attrs[1])) {
        valid = l;
    }
    else {
        DIE();
    }

    /* The second attribute must be disallow */
    if(strcmp("disallow", attrs[2])) {
        DIE();
    }

    /* Create a temporary copy of the string for parsing */
    str = strdup(attrs[3]);
    tok = strtok_r(str, ", ", &lasts);

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
            free(str);
            DIE();
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

    /* Clean up the temporary string, since we're done */
    free(str);
}

static void parse_colors(const XML_Char **attrs, uint16_t *valid) {
    int i;
    char *str, *lasts, *tok;

    /* Make sure we have a sane set */
    if(!attrs || !attrs[0] || !attrs[1] || attrs[2]) {
        DIE();
    }

    /* The only valid attribute here is disallow */
    if(strcmp("disallow", attrs[0])) {
        DIE();
    }

    /* Create a temporary copy of the string for parsing */
    str = strdup(attrs[1]);
    tok = strtok_r(str, ", ", &lasts);

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
            free(str);
            DIE();
        }

        /* Grab the next token, and check for it */
        tok = strtok_r(NULL, ", ", &lasts);
    }

    /* Clean up the temporary string, since we're done */
    free(str);
}

static void handle_weapon(const XML_Char *name, const XML_Char **attrs) {
    sylverant_weapon_t *w = (sylverant_weapon_t *)cur_item;

    if(!strcmp(name, "grind")) {
        parse_max(attrs, &w->max_grind, &w->min_grind);
    }
    else if(!strcmp(name, "percents")) {
        parse_max(attrs, &w->max_percents, &w->min_percents);
    }
    else if(!strcmp(name, "attributes")) {
        parse_attrs(attrs, &w->valid_attrs);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_frame(const XML_Char *name, const XML_Char **attrs) {
    sylverant_frame_t *f = (sylverant_frame_t *)cur_item;

    if(!strcmp(name, "slots")) {
        parse_max(attrs, &f->max_slots, &f->min_slots);
    }
    else if(!strcmp(name, "dfp")) {
        parse_max(attrs, &f->max_dfp, &f->min_dfp);
    }
    else if(!strcmp(name, "evp")) {
        parse_max(attrs, &f->max_evp, &f->min_evp);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_barrier(const XML_Char *name, const XML_Char **attrs) {
    sylverant_barrier_t *b = (sylverant_barrier_t *)cur_item;

    if(!strcmp(name, "dfp")) {
        parse_max(attrs, &b->max_dfp, &b->min_dfp);
    }
    else if(!strcmp(name, "evp")) {
        parse_max(attrs, &b->max_evp, &b->min_evp);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_unit(const XML_Char *name, const XML_Char **attrs) {
    sylverant_unit_t *u = (sylverant_unit_t *)cur_item;

    if(!strcmp(name, "plus")) {
        parse_max(attrs, &u->max_plus, &u->min_plus);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_mag(const XML_Char *name, const XML_Char **attrs) {
    sylverant_mag_t *m = (sylverant_mag_t *)cur_item;

    if(!strcmp(name, "level")) {
        parse_max(attrs, &m->max_level, &m->min_level);
    }
    else if(!strcmp(name, "def")) {
        parse_max(attrs, &m->max_def, &m->min_def);
    }
    else if(!strcmp(name, "pow")) {
        parse_max(attrs, &m->max_pow, &m->min_pow);
    }
    else if(!strcmp(name, "dex")) {
        parse_max(attrs, &m->max_dex, &m->min_dex);
    }
    else if(!strcmp(name, "mind")) {
        parse_max(attrs, &m->max_mind, &m->min_mind);
    }
    else if(!strcmp(name, "synchro")) {
        parse_max(attrs, &m->max_synchro, &m->min_synchro);
    }
    else if(!strcmp(name, "iq")) {
        parse_max(attrs, &m->max_iq, &m->min_iq);
    }
    else if(!strcmp(name, "pbs")) {
        parse_pbs(attrs, &m->allowed_cpb, &m->allowed_rpb, &m->allowed_lpb);
    }
    else if(!strcmp(name, "colors")) {
        parse_colors(attrs, &m->allowed_colors);
    }
    else if(!common_tag(name, attrs)) {
        DIE();
    }
}

static void handle_tool(const XML_Char *name, const XML_Char **attrs) {
    sylverant_tool_t *t = (sylverant_tool_t *)cur_item;

    if(!strcmp(name, "stack")) {
        parse_max(attrs, &t->max_stack, &t->min_stack);
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

            memset(w, 0xFF, sizeof(sylverant_weapon_t));

            cur_item = (sylverant_item_t *)w;
            cur_hnd = &handle_weapon;
            w->base.item_code = code;
            w->valid_attrs = FULL_ATTR_VALID;
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

                    memset(f, 0xFF, sizeof(sylverant_frame_t));

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

                    memset(b, 0xFF, sizeof(sylverant_barrier_t));

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

                    memset(u, 0xFF, sizeof(sylverant_unit_t));

                    cur_item = (sylverant_item_t *)u;
                    cur_hnd = &handle_unit;
                    u->base.item_code = code;

                    /* Since this can be -1 or -2, set it to an off-the-wall
                       value. */
                    u->max_plus = INT_MIN;
                    u->min_plus = INT_MIN;
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

            memset(m, 0xFF, sizeof(sylverant_mag_t));

            cur_item = (sylverant_item_t *)m;
            cur_hnd = &handle_mag;
            m->base.item_code = code;
            m->allowed_colors = l->default_colors;
            m->allowed_cpb = l->default_cpb;
            m->allowed_rpb = l->default_rpb;
            m->allowed_lpb = l->default_lpb;
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

            memset(t, 0xFF, sizeof(sylverant_tool_t));

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

    /* Don't auto-reject items unless we hit that tag. */
    cur_item->auto_reject = 0;
}

static void item_start_hnd(void *d, const XML_Char *name,
                           const XML_Char **attrs) {
    sylverant_limits_t *l = *(sylverant_limits_t **)d;

    if(!strcmp(name, "items")) {
        handle_items(l, attrs);
    }
    else if(!strcmp(name, "pbs") && in_items && !cur_item) {
        parse_pbs(attrs, &l->default_cpb, &l->default_rpb, &l->default_lpb);
    }
    else if(!strcmp(name, "colors") && in_items && !cur_item) {
        parse_colors(attrs, &l->default_colors);
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

    /* Set the default behavior for photon blasts/colors */
    rv->default_cpb = rv->default_rpb = rv->default_lpb = 0xFF;
    rv->default_colors = 0xFFFF;

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
            printf("%s\n", XML_ErrorString(XML_GetErrorCode(p)));
            printf("\tAt: %d:%d\n", (int)XML_GetCurrentLineNumber(p),
                   (int)XML_GetCurrentColumnNumber(p));
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
            printf("Error reading file\n");
            printf("\tAt: %d:%d\n", (int)XML_GetCurrentLineNumber(p),
                   (int)XML_GetCurrentColumnNumber(p));
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
            printf("%s\n", XML_ErrorString(XML_GetErrorCode(p)));
            printf("\tAt: %d:%d\n", (int)XML_GetCurrentLineNumber(p),
                   (int)XML_GetCurrentColumnNumber(p));
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
                        uint32_t version, uint32_t ic) {
    sylverant_item_t *j;
    sylverant_weapon_t *w;
    int is_srank = 0, is_named_srank = 0;
    uint8_t tmp;

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

            /* Check the grind value first */
            if((w->max_grind != -1 && i->data_b[3] > w->max_grind) ||
               (w->min_grind != -1 && i->data_b[3] < w->min_grind)) {
                return 0;
            }

            /* Check each percent */
            if(!is_named_srank) {
                if(w->max_percents != -1) {
                    if((i->data_b[6] && i->data_b[7] > w->max_percents) ||
                       (i->data_b[8] && i->data_b[9] > w->max_percents) ||
                       (i->data_b[10] && i->data_b[11] > w->max_percents)) {
                        return 0;
                    }
                }

                if(w->min_percents != -1) {
                    if((i->data_b[6] && i->data_b[7] < w->min_percents) ||
                       (i->data_b[8] && i->data_b[9] < w->min_percents) ||
                       (i->data_b[10] && i->data_b[11] < w->min_percents)) {
                        return 0;
                    }
                }
            }

            /* Check if the attribute of the weapon is valid */
            if(i->data_b[4] > Weapon_Attr_MAX) {
                return 0;
            }

            if(!(w->valid_attrs & (1 << i->data_b[4]))) {
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
    uint16_t tmp;
    int16_t tmp2;
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
                    tmp = i->data_b[6] | (i->data_b[7] << 8);
                    if((f->max_dfp != -1 && tmp > f->max_dfp) ||
                       (f->min_dfp != -1 && tmp < f->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    tmp = i->data_b[8] | (i->data_b[9] << 8);
                    if((f->max_evp != -1 && tmp > f->max_evp) ||
                       (f->min_evp != -1 && tmp < f->min_evp)) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_BARRIER:
                    b = (sylverant_barrier_t *)j;

                    /* Check if the dfp boost is too high */
                    tmp = i->data_b[6] | (i->data_b[7] << 8);
                    if((b->max_dfp != -1 && tmp > b->max_dfp) ||
                       (b->min_dfp != -1 && tmp < b->min_dfp)) {
                        return 0;
                    }

                    /* Check if the evp boost is too high */
                    tmp = i->data_b[8] | (i->data_b[9] << 8);
                    if((b->max_evp != -1 && tmp > b->max_evp) ||
                       (b->min_evp != -1 && tmp < b->min_evp)) {
                        return 0;
                    }

                    break;

                case ITEM_SUBTYPE_UNIT:
                    u = (sylverant_unit_t *)j;

                    /* Check the Plus/Minus number */
                    tmp2 = i->data_b[6] | (i->data_b[7] << 8);
                    if((u->max_plus != INT_MIN && tmp2 > u->max_plus) ||
                       (u->min_plus != INT_MIN && tmp2 < u->min_plus)) {
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
