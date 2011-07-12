/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2011 Lawrence Sebald

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

#ifndef SYLVERANT__CHARACTERS_H
#define SYLVERANT__CHARACTERS_H

#include <stdint.h>

#ifdef PACKED
#undef PACKED
#endif

#define PACKED __attribute__((packed))

typedef struct sylverant_iitem {
    uint16_t equipped;
    uint16_t tech;
    uint32_t flags;
    
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
} PACKED sylverant_iitem_t;

typedef struct sylverant_bitem {
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

    uint16_t amount;
    uint16_t flags;
} PACKED sylverant_bitem_t;

typedef struct sylverant_inventory {
    uint8_t item_count;
    uint8_t hpmats_used;
    uint8_t tpmats_used;
    uint8_t language;
    sylverant_iitem_t items[30];
} PACKED sylverant_inventory_t;

typedef struct sylverant_bank {
    uint32_t item_count;
    uint32_t meseta;
    sylverant_bitem_t items[200];
} PACKED sylverant_bank_t;

/* The version of the player data that goes out to other lobby members, minus
   the inventory. */
typedef struct sylverant_bb_char {
    uint16_t atp;
    uint16_t mst;
    uint16_t evp;
    uint16_t hp;
    uint16_t dfp;
    uint16_t ata;
    uint16_t lck;
    uint16_t unk1;
    uint32_t unk2[2];
    uint32_t level;
    uint32_t exp;
    uint32_t meseta;
    char guildcard_str[16];
    uint32_t unk3[2];
    uint32_t name_color;
    uint8_t model;
    uint8_t unused[11];
    uint32_t play_time;                 /* Placed here, like newserv */
    uint32_t name_color_checksum;
    uint8_t section;
    uint8_t ch_class;
    uint8_t v2flags;
    uint8_t version;
    uint32_t v1flags;
    uint16_t costume;
    uint16_t skin;
    uint16_t face;
    uint16_t head;
    uint16_t hair;
    uint16_t hair_r;
    uint16_t hair_g;
    uint16_t hair_b;
    float prop_x;
    float prop_y;
    uint16_t name[16];
    uint8_t config[0xE8];
    uint8_t techniques[0x14];
} PACKED sylverant_bb_char_t;

/* This version of the character data is used for the character select screen */
typedef struct sylverant_bb_mini_char {
    uint32_t exp;
    uint32_t level;
    char guildcard_str[16];
    uint32_t unk3[2];                   /* Named to match other structs */
    uint32_t name_color;
    uint8_t model;
    uint8_t unused[15];
    uint32_t name_color_checksum;
    uint8_t section;
    uint8_t ch_class;
    uint8_t v2flags;
    uint8_t version;
    uint32_t v1flags;
    uint16_t costume;
    uint16_t skin;
    uint16_t face;
    uint16_t head;
    uint16_t hair;
    uint16_t hair_r;
    uint16_t hair_g;
    uint16_t hair_b;
    float prop_x;
    float prop_y;
    uint16_t name[16];
    uint32_t play_time;
} PACKED sylverant_bb_mini_char_t;

/* Blue Burst team data and key configuration */
typedef struct sylverant_bb_key_team_config {
    uint8_t unk[0x114];
    uint8_t key_config[0x16C];
    uint8_t joystick_config[0x38];
    uint32_t guildcard;
    uint32_t team_id;
    uint32_t team_info[2];
    uint16_t team_priv;
    uint16_t reserved;
    uint16_t team_name[16];
    uint8_t team_flag[2048];
    uint32_t team_rewards[2];
} PACKED sylverant_bb_key_team_config_t;

/* Blue Burst player data */
typedef struct sylverant_bb_player {
    sylverant_inventory_t inv;
    sylverant_bb_char_t character;
    uint8_t c_rank[0x0174];
    uint16_t infoboard[172];
    uint32_t blacklist[30];
    uint32_t autoreply_enabled;
    uint16_t autoreply[];
} PACKED sylverant_bb_player_t;

/* Blue Burst full character data  */
typedef struct sylverant_bb_full_char {
    sylverant_inventory_t inv;
    sylverant_bb_char_t character;
    uint8_t unk[16];
    uint32_t option_flags;
    uint8_t quest_data1[520];
    sylverant_bank_t bank;
    uint32_t guildcard;
    uint16_t name[24];
    uint16_t team_name[16];
    uint16_t guildcard_desc[88];
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t section;
    uint8_t ch_class;
    uint32_t unk2;
    uint8_t symbol_chats[1248];
    uint8_t shortcuts[2624];
    uint16_t autoreply[172];
    uint16_t infoboard[172];
    uint8_t unk3[28];
    uint8_t challenge_data[320];
    uint8_t tech_menu[40];
    uint8_t unk4[44];
    uint8_t quest_data2[88];
    sylverant_bb_key_team_config_t cfg;
} PACKED sylverant_bb_full_char_t;

/* Subset of the full character data that's stored to the db in the
   character_data table. This is modeled after newserv's .nsc files. */
typedef struct sylverant_bb_db_char {
    sylverant_inventory_t inv;
    sylverant_bb_char_t character;
    uint8_t quest_data1[520];
    sylverant_bank_t bank;
    uint16_t guildcard_desc[88];
    uint16_t autoreply[172];
    uint16_t infoboard[172];
    uint8_t challenge_data[320];
    uint8_t tech_menu[40];
    uint8_t quest_data2[88];
} PACKED sylverant_bb_db_char_t;

/* Subset of the full character data that's stored in the db in the
   blueburst_options table. This is modeled after newserv's .nsa files. */
typedef struct sylverant_bb_db_opts {
    uint32_t blocked[30];
    uint8_t key_config[0x16C];
    uint8_t joystick_config[0x38];
    uint32_t option_flags;
    uint8_t shortcuts[0xA40];
    uint8_t symbol_chats[0x4E0];
    uint16_t team_name[16];
} PACKED sylverant_bb_db_opts_t;

#undef PACKED

#endif /* !SYLVERANT__CHARACTERS_H */
