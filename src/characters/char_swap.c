/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009 Lawrence Sebald

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

#include <stdio.h>
#include "sylverant/characters.h"

#if defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__)
#define LE16(x) (((x >> 8) & 0xFF) | ((x & 0xFF) << 8))
#define LE32(x) (((x >> 24) & 0x00FF) | \
                 ((x >>  8) & 0xFF00) | \
                 ((x & 0xFF00) <<  8) | \
                 ((x & 0x00FF) << 24))
#define LE64(x) (((x >> 56) & 0x000000FF) | \
                 ((x >> 40) & 0x0000FF00) | \
                 ((x >> 24) & 0x00FF0000) | \
                 ((x >>  8) & 0xFF000000) | \
                 ((x & 0xFF000000) <<  8) | \
                 ((x & 0x00FF0000) << 24) | \
                 ((x & 0x0000FF00) << 40) | \
                 ((x & 0x000000FF) << 56))

static inline void swap_item(sylverant_item_t *item) {
    /* If its not a mag, this is easy. */
    if(item->data[0] != 2) {
        item->itemid = LE32(item->itemid);
    }
    else {
        sylverant_mag_t *mag = (sylverant_mag_t *)item;

        mag->defense = LE16(mag->defense);
        mag->power = LE16(mag->power);
        mag->dex = LE16(mag->dex);
        mag->mind = LE16(mag->mind);
        mag->itemid = LE32(mag->itemid);
    }
}

/* Ugh... this isn't pretty, but it should work. */
void sylverant_char_swap(sylverant_character_t *ch) {
    int i;

    /* Fix everything except items first. */
    ch->packetSize = LE16(ch->packetSize);
    ch->command = LE16(ch->command);
    ch->ATP = LE16(ch->ATP);
    ch->MST = LE16(ch->MST);
    ch->EVP = LE16(ch->EVP);
    ch->HP = LE16(ch->HP);
    ch->DFP = LE16(ch->DFP);
    ch->TP = LE16(ch->TP);
    ch->LCK = LE16(ch->LCK);
    ch->ATA = LE16(ch->ATA);
    ch->level = LE16(ch->level);
    ch->unknown2 = LE16(ch->unknown2);
    ch->XP = LE32(ch->XP);
    ch->meseta = LE32(ch->meseta);
    ch->skinID = LE16(ch->skinID);
    ch->costume = LE16(ch->costume);
    ch->skin = LE16(ch->skin);
    ch->face = LE16(ch->face);
    ch->head = LE16(ch->head);
    ch->hair = LE16(ch->hair);
    ch->hairColorRed = LE16(ch->hairColorRed);
    ch->hairColorBlue = LE16(ch->hairColorBlue);
    ch->hairColorGreen = LE16(ch->hairColorGreen);
    ch->proportionX = LE32(ch->proportionX);
    ch->proportionY = LE32(ch->proportionY);
    ch->playTime = LE32(ch->playTime);
    ch->bankUse = LE32(ch->bankUse);
    ch->bankMeseta = LE32(ch->bankMeseta);
    ch->guildCard = LE32(ch->guildCard);
    ch->guildCard2 = LE32(ch->guildCard2);
    ch->teamID = LE32(ch->teamID);
    ch->privilegeLevel = LE16(ch->privilegeLevel);
    ch->reserved3 = LE16(ch->reserved3);
    ch->unknown15 = LE32(ch->unknown15);

    /* Fix up items in the base inventory. */
    for(i = 0; i < 30; ++i) {
        ch->inventory[i].in_use = LE32(ch->inventory[i].in_use);
        ch->inventory[i].flags = LE32(ch->inventory[i].flags);
        swap_item(&ch->inventory[i].item);
    }

    /* Fix up items in the bank inventory. */
    for(i = 0; i < 200; ++i) {
        swap_item((sylverant_item_t *)&ch->bankInventory[i]);
        ch->bankInventory[i].bank_count = LE32(ch->bankInventory[i].bank_count);
    }
}

#else

void sylverant_char_swap(sylverant_character_t *ch) {
    /* Nothing here... We should already be in little-endian. */
}

#endif


