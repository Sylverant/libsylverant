/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009 Lawrence Sebald
    Copyright (C) 2008 Terry Chatman Jr.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SYLVERANT__CHARACTERS_H
#define SYLVERANT__CHARACTERS_H

#include <inttypes.h>
#include "sylverant/database.h"

/* The following few structures were taken from the Tethealla PSO server's code
   (Specifically from the login_server), which is available under the GPLv3. The
   structures have been renamed to fit more into the Sylverant naming structure,
   and comments may have been reformatted a bit, also, some types have been
   changed to those in inttypes.h for safety. */

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/* Mag data structure */
typedef struct sylverant_mag {
    uint8_t two;                       // "02" =P
    uint8_t mtype;
    uint8_t level;
    uint8_t blasts;
    int16_t defense;
    int16_t power;
    int16_t dex;
    int16_t mind;
    uint32_t itemid;
    char synchro;
    uint8_t IQ;
    uint8_t PBflags;
    uint8_t color;
} PACKED sylverant_mag_t;

/* Mini-character Data Structure */
typedef struct sylverant_mini_char {
    uint16_t packetSize;               // 0x00 - 0x01
    uint16_t command;                  // 0x02 - 0x03
    uint8_t flags[4];                  // 0x04 - 0x07
    uint8_t unknown[8];                // 0x08 - 0x0F
    uint16_t level;                    // 0x10 - 0x11
    uint16_t reserved;                 // 0x12 - 0x13
    char gcString[10];                 // 0x14 - 0x1D
    uint8_t unknown2[14];              // 0x1E - 0x2B
    uint8_t nameColorBlue;             // 0x2C
    uint8_t nameColorGreen;            // 0x2D
    uint8_t nameColorRed;              // 0x2E
    uint8_t nameColorTransparency;     // 0x2F
    uint16_t skinID;                   // 0x30 - 0x31
    uint8_t unknown3[18];              // 0x32 - 0x43
    uint8_t sectionID;                 // 0x44
    uint8_t _class;                    // 0x45
    uint8_t skinFlag;                  // 0x46
    uint8_t unknown4[5];               // 0x47 - 0x4B (same as unknown5 in E7)
    uint16_t costume;                  // 0x4C - 0x4D
    uint16_t skin;                     // 0x4E - 0x4F
    uint16_t face;                     // 0x50 - 0x51
    uint16_t head;                     // 0x52 - 0x53
    uint16_t hair;                     // 0x54 - 0x55
    uint16_t hairColorRed;             // 0x56 - 0x57
    uint16_t hairColorBlue;            // 0x58 - 0x59
    uint16_t hairColorGreen;           // 0x5A - 0x5B
    uint32_t proportionX;              // 0x5C - 0x5F
    uint32_t proportionY;              // 0x60 - 0x63
    uint8_t name[24];                  // 0x64 - 0x7B
    uint8_t unknown5[8];               // 0x7C - 0x83
    uint32_t playTime;
} PACKED sylverant_mini_char_t;

typedef struct sylverant_bank_item {
    uint8_t data[12];                  // the standard $setitem1 - $setitem3
                                       // fare
    uint32_t itemid;                   // player item id
    uint8_t data2[4];                  // $setitem4 (mag use only)
    uint32_t bank_count;               // Why?
} PACKED sylverant_bank_item_t;

typedef struct sylverant_item {
    uint8_t data[12];                  // the standard $setitem1 - $setitem3
                                       // fare
    uint32_t itemid;                   // player item id
    uint8_t data2[4];                  // $setitem4 (mag use only)
} PACKED sylverant_item_t;

typedef struct sylverant_inventory {
    uint32_t in_use;                   // 0x01 = item slot in use, 0xFF00 =
                                       // unused
    uint32_t flags;                    // 8 = equipped
    sylverant_item_t item;
} PACKED sylverant_inventory_t;

/* Full character data structure. */
typedef struct sylverant_character {
    uint16_t packetSize;               // 0x00-0x01 // Always set to 0x399C
    uint16_t command;                  // 0x02-0x03 // Always set to 0x00E7
    uint8_t flags[4];                  // 0x04-0x07
    uint8_t inventoryUse;              // 0x08
    uint8_t HPuse;                     // 0x09
    uint8_t TPuse;                     // 0x0A
    uint8_t lang;                      // 0x0B
    sylverant_inventory_t inventory[30];    // 0x0C-0x353
    uint16_t ATP;                      // 0x354-0x355
    uint16_t MST;                      // 0x356-0x357
    uint16_t EVP;                      // 0x358-0x359
    uint16_t HP;                       // 0x35A-0x35B
    uint16_t DFP;                      // 0x35C-0x35D
    uint16_t TP;                       // 0x35E-0x35F
    uint16_t LCK;                      // 0x360-0x361
    uint16_t ATA;                      // 0x362-0x363
    uint8_t unknown[8];                // 0x364-0x36B (Offset 0x360 has 0x0A
                                       // value on Schthack's...)
    uint16_t level;                    // 0x36C-0x36D;
    uint16_t unknown2;                 // 0x36E-0x36F;
    uint32_t XP;                       // 0x370-0x373
    uint32_t meseta;                   // 0x374-0x377;
    char gcString[10];                 // 0x378-0x381;
    uint8_t unknown3[14];              // 0x382-0x38F; // Same as E5 unknown2
    uint8_t nameColorBlue;             // 0x390;
    uint8_t nameColorGreen;            // 0x391;
    uint8_t nameColorRed;              // 0x392;
    uint8_t nameColorTransparency;     // 0x393;
    uint16_t skinID;                   // 0x394-0x395;
    uint8_t unknown4[18];              // 0x396-0x3A7
    uint8_t sectionID;                 // 0x3A8;
    uint8_t _class;                    // 0x3A9;
    uint8_t skinFlag;                  // 0x3AA;
    uint8_t unknown5[5];               // 0x3AB-0x3AF; // Same as E5 unknown4.
    uint16_t costume;                  // 0x3B0 - 0x3B1;
    uint16_t skin;                     // 0x3B2 - 0x3B3;
    uint16_t face;                     // 0x3B4 - 0x3B5;
    uint16_t head;                     // 0x3B6 - 0x3B7;
    uint16_t hair;                     // 0x3B8 - 0x3B9;
    uint16_t hairColorRed;             // 0x3BA-0x3BB;
    uint16_t hairColorBlue;            // 0x3BC-0x3BD;
    uint16_t hairColorGreen;           // 0x3BE-0x3BF;
    uint32_t proportionX;              // 0x3C0-0x3C3;
    uint32_t proportionY;              // 0x3C4-0x3C7;
    uint8_t name[24];                  // 0x3C8-0x3DF;
    uint32_t playTime;                 // 0x3E0 - 0x3E3
    uint8_t unknown6[4];               // 0x3E4 - 0x3E7;
    uint8_t keyConfig[232];            // 0x3E8 - 0x4CF;
                                       // Stored from ED 07 packet.
    uint8_t techniques[20];            // 0x4D0 - 0x4E3;
    uint8_t unknown7[16];              // 0x4E4 - 0x4F3;
    uint8_t options[4];                // 0x4F4-0x4F7;
                                       // Stored from ED 01 packet.
    uint8_t unknown8[520];             // 0x4F8 - 0x6FF;
    uint32_t bankUse;                  // 0x700 - 0x703
    uint32_t bankMeseta;               // 0x704 - 0x707;
    sylverant_bank_item_t bankInventory[200];   // 0x708 - 0x19C7
    uint32_t guildCard;                // 0x19C8-0x19CB;
                                       // Stored from E8 06 packet.
    uint8_t name2[24];                 // 0x19CC - 0x19E3;
    uint8_t unknown9[232];             // 0x19E4-0x1ACB;
    uint8_t reserved1;                 // 0x1ACC; // Has value 0x01 on
                                       // Schthack's
    uint8_t reserved2;                 // 0x1ACD; // Has value 0x01 on
                                       // Schthack's
    uint8_t sectionID2;                // 0x1ACE;
    uint8_t _class2;                   // 0x1ACF;
    uint8_t unknown10[4];              // 0x1AD0-0x1AD3;
    uint8_t symbol_chats[1248];        // 0x1AD4 - 0x1FB3
                                       // Stored from ED 02 packet.
    uint8_t shortcuts[2624];           // 0x1FB4 - 0x29F3
                                       // Stored from ED 03 packet.
    uint8_t unknown11[344];            // 0x29F4 - 0x2B4B;
    uint8_t GCBoard[172];              // 0x2B4C - 0x2BF7;
    uint8_t unknown12[200];            // 0x2BF8 - 0x2CBF;
    uint8_t challengeData[320];        // 0x2CC0 - 0X2DFF
    uint8_t unknown13[172];            // 0x2E00 - 0x2EAB;
    uint8_t unknown14[276];            // 0x2EAC - 0x2FBF; // I don't know what
                                       // this is, but split from unknown13
                                       // because this chunk is actually copied
                                       // into the 0xE2 packet during login @
                                       // 0x08
    uint8_t keyConfigGlobal[364];      // 0x2FC0 - 0x312B // Copied into 0xE2
                                       // login packet @ 0x11C
                                       // Stored from ED 04 packet.
    uint8_t joyConfigGlobal[56];       // 0x312C - 0x3163 // Copied into 0xE2
                                       // login packet @ 0x288
                                       // Stored from ED 05 packet.
    uint32_t guildCard2;               // 0x3164 - 0x3167 (From here on copied
                                       // into 0xE2 login packet @ 0x2C0...)
    uint32_t teamID;                   // 0x3168 - 0x316B
    uint8_t teamInformation[8];        // 0x316C - 0x3173 (usually blank...)
    uint16_t privilegeLevel;           // 0x3174 - 0x3175
    uint16_t reserved3;                // 0x3176 - 0x3177
    uint8_t teamName[28];              // 0x3178 - 0x3193
    uint32_t unknown15;                // 0x3194 - 0x3197
    uint8_t teamFlag[2048];            // 0x3198 - 0x3997
    uint8_t teamRewards[8];            // 0x3998 - 0x39A0
} PACKED sylverant_character_t;

#undef PACKED

/* Character classes, taken from Tethealla's Login Server. */
#define CLASS_HUMAR     0x00
#define CLASS_HUNEWEARL 0x01
#define CLASS_HUCAST    0x02
#define CLASS_RAMAR     0x03
#define CLASS_RACAST    0x04
#define CLASS_RACASEAL  0x05
#define CLASS_FOMARL    0x06
#define CLASS_FONEWM    0x07
#define CLASS_FONEWEARL 0x08
#define CLASS_HUCASEAL  0x09
#define CLASS_FOMAR     0x0A
#define CLASS_RAMARL    0x0B
#define CLASS_MAX       0x0C

/* Functions for working with characters. */
extern int sylverant_char_fetch(sylverant_dbconn_t *conn, uint32_t gc, int slot,
                                sylverant_character_t *rv);

extern int sylverant_char_fetch_mini(sylverant_dbconn_t *conn, uint32_t gc,
                                     int slot, sylverant_mini_char_t *rv);

extern int sylverant_char_create(sylverant_dbconn_t *conn, uint32_t gc,
                                 int slot, sylverant_mini_char_t *ch,
                                 const void *e7_base,
                                 const uint8_t *starting_stats);

extern void sylverant_char_sanitize(sylverant_mini_char_t *ch);

extern int sylverant_char_update(sylverant_dbconn_t *conn, uint32_t gc,
                                 int slot, sylverant_mini_char_t *ch);

extern int sylverant_char_update_full(sylverant_dbconn_t *conn, uint32_t gc,
                                      int slot, sylverant_character_t *ch);

/* Swap the endianness of a character (on big-endian only). This is a no-op
   on a little-endian machine. */
extern void sylverant_char_swap(sylverant_character_t *ch);

#endif /* !SYLVERANT__CHARACTERS_H */
