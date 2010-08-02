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

#ifndef SYLVERANT__ITEMS_H
#define SYLVERANT__ITEMS_H

#include <inttypes.h>
#include <sys/queue.h>

/* Item buckets. Each item gets put into one of these buckets when in the list,
   in order to make searching the list slightly easier. These are all based on
   the least significant byte of the item code. */
#define ITEM_TYPE_WEAPON        0x00
#define ITEM_TYPE_GUARD         0x01
#define ITEM_TYPE_MAG           0x02
#define ITEM_TYPE_TOOL          0x03

/* ITEM_TYPE_GUARD items are actually slightly more specialized, and fall into
   three subtypes of their own. These are the second least significant byte in
   the item code. */
#define ITEM_SUBTYPE_FRAME      0x01
#define ITEM_SUBTYPE_BARRIER    0x02
#define ITEM_SUBTYPE_UNIT       0x03

/* Default behaviors for the item lists. ITEM_DEFAULT_ALLOW means to accept any
   things NOT in the list read in by default, whereas ITEM_DEFAULT_REJECT causes
   unlisted items to be rejected. */
#define ITEM_DEFAULT_ALLOW      1
#define ITEM_DEFAULT_REJECT     0

/* Version codes, as used in this part of the code. */
#define ITEM_VERSION_V1         0x01
#define ITEM_VERSION_V2         0x02
#define ITEM_VERSION_GC         0x04

/* Some combinations of the above. */
#define ITEM_VERSION_DC_ONLY    0x03
#define ITEM_VERSION_2_AND_UP   0xFE
#define ITEM_VERSION_ALL        0xFF

/* Base item structure. This is not generally used directly, but rather as a
   piece of the overall puzzle. */
typedef struct sylverant_item {
    TAILQ_ENTRY(sylverant_item) qentry;

    uint32_t item_code;
    uint32_t allowed_versions;
    int auto_reject;
} sylverant_item_t;

TAILQ_HEAD(sylverant_item_queue, sylverant_item);

/* Weapon information structure. This is a "subclass" of the above item struct
   which holds information specific to weapons. */
typedef struct sylverant_weapon {
    sylverant_item_t base;

    int max_grind;
    int min_grind;
    int max_percents;
    int min_percents;
} sylverant_weapon_t;

/* Frame information structure. */
typedef struct sylverant_frame {
    sylverant_item_t base;

    int max_slots;
    int min_slots;
    int max_dfp;
    int min_dfp;
    int max_evp;
    int min_evp;
} sylverant_frame_t;

/* Barrier information structure. */
typedef struct sylverant_barrier {
    sylverant_item_t base;

    int max_dfp;
    int min_dfp;
    int max_evp;
    int min_evp;
} sylverant_barrier_t;

/* Unit information structure. */
typedef struct sylverant_unit {
    sylverant_item_t base;

    int max_plus;
    int min_plus;
} sylverant_unit_t;

/* Mag information structure. */
typedef struct sylverant_mag {
    sylverant_item_t base;

    int max_level;
    int min_level;
    int max_def;
    int min_def;
    int max_pow;
    int min_pow;
    int max_dex;
    int min_dex;
    int max_mind;
    int min_mind;
    int max_synchro;
    int min_synchro;
    int max_iq;
    int min_iq;
} sylverant_mag_t;

/* Tool information structure. */
typedef struct sylverant_tool {
    sylverant_item_t base;

    int max_stack;
    int min_stack;
} sylverant_tool_t;

/* Overall list for reading in the configuration.*/
typedef struct sylverant_limits {
    struct sylverant_item_queue *weapons;
    struct sylverant_item_queue *guards;
    struct sylverant_item_queue *mags;
    struct sylverant_item_queue *tools;

    int default_behavior;
} sylverant_limits_t;

/* Raw inventory item data */
typedef struct sylverant_iitem {
    uint32_t flags[2];

    union {
        uint8_t data_b[12];
        uint16_t data_w[6];
        uint32_t data_l[3];
    };

    uint32_t item_id;

    union {
        uint8_t data2_b[4];
        uint16_t data2_w[2];
        uint32_t data2_l;
    };
} __attribute__((packed)) sylverant_iitem_t;

/* Read the item limits data. You are responsible for calling the function to
   clean everything up when you're done. */
extern int sylverant_read_limits(const char *f, sylverant_limits_t **l);

/* Clean up the limits data. */
extern int sylverant_free_limits(sylverant_limits_t *l);

/* Find an item in the limits list, if its there, and check for legitness.
   Returns non-zero if the item is legit. */
extern int sylverant_limits_check_item(sylverant_limits_t *l,
                                       sylverant_iitem_t *i);

#endif /* !SYLVERANT__ITEMS_H */
