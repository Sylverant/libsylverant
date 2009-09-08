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

#include <stdio.h>
#include <string.h>
#include "sylverant/characters.h"

/* Maximum length of a SQL query to put a character in the DB. */
#define SZ sizeof(sylverant_character_t) * 2 + \
    sizeof(sylverant_mini_char_t) * 2 + 256

/* Most of this code is adapted from the Tethealla Login Server's
   AckCharacter_Creation function. */
int sylverant_char_create(sylverant_dbconn_t *conn, uint32_t gc, int slot,
                          sylverant_mini_char_t *ch, const void *e7_base,
                          const uint8_t *starting_stats) {
    sylverant_character_t newchar, *base;
    char query[SZ];
    int i;
    unsigned short *n;

    /* Check the input. */
    if(!conn || slot < 0 || slot > 3 || !ch || !e7_base || !starting_stats) {
        return -42;
    }

    /* Do some setup work. */
    base = (sylverant_character_t *)e7_base;
    memset(&newchar, 0, sizeof(sylverant_character_t));

    newchar.packetSize = 0x399C;
    newchar.command = 0x00E7;
    newchar.HPuse = 0;
    newchar.TPuse = 0;
    newchar.lang = 1;

    /* Set up the character's initial weapon. */
    switch(ch->_class) {
        case CLASS_HUMAR:
        case CLASS_HUNEWEARL:
        case CLASS_HUCAST:
        case CLASS_HUCASEAL:
            /* Give hunters a Saber. */
            newchar.inventory[0].in_use = 0x01;
            newchar.inventory[0].flags = 0x08;
            newchar.inventory[0].item.data[1] = 0x01;
            newchar.inventory[0].item.itemid = 0x00010000;
            break;

        case CLASS_RAMAR:
        case CLASS_RACAST:
        case CLASS_RACASEAL:
        case CLASS_RAMARL:
            /* Give rangers a Handgun */
            newchar.inventory[0].in_use = 0x01;
            newchar.inventory[0].flags = 0x08;
            newchar.inventory[0].item.data[1] = 0x06;
            newchar.inventory[0].item.itemid = 0x00010000;
            break;

        case CLASS_FONEWM:
        case CLASS_FONEWEARL:
        case CLASS_FOMARL:
        case CLASS_FOMAR:
            /* Give forces a Cane */
            newchar.inventory[0].in_use = 0x01;
            newchar.inventory[0].flags = 0x08;
            newchar.inventory[0].item.data[1] = 0x0A;
            newchar.inventory[0].item.itemid = 0x00010000;
            break;
    }
    
    /* Everyone gets a Frame. */
    newchar.inventory[1].in_use = 0x01;
    newchar.inventory[1].flags = 0x08;
    newchar.inventory[1].item.data[0] = 0x01;
    newchar.inventory[1].item.data[1] = 0x01;
    newchar.inventory[1].item.itemid = 0x00010001;

    /* Everyone gets a Mag. */
    newchar.inventory[2].in_use = 0x01;
    newchar.inventory[2].flags = 0x08;
    newchar.inventory[2].item.data[0] = 0x02;
    newchar.inventory[2].item.data[2] = 0x05;
    newchar.inventory[2].item.data[4] = 0xF4;
    newchar.inventory[2].item.data[5] = 0x01;
    newchar.inventory[2].item.data2[0] = 0x14;   // 20% synchro
    newchar.inventory[2].item.itemid = 0x00010002;

    /* Set the Mag color as appropriate. */
    if((ch->_class == CLASS_HUCAST) || (ch->_class == CLASS_HUCASEAL) ||
       (ch->_class == CLASS_RACAST) || (ch->_class == CLASS_RACASEAL)) {
        newchar.inventory[2].item.data2[3] = (unsigned char)ch->skin;
    }
    else {
        newchar.inventory[2].item.data2[3] = (unsigned char)ch->costume;
    }

    /* If the Mag color is above the max, fix it. */
    if(newchar.inventory[2].item.data2[3] > 0x11)
        newchar.inventory[2].item.data2[3] -= 0x11;

    /* Everyone starts with some Monomates. */
    newchar.inventory[3].in_use = 0x01;
    newchar.inventory[3].item.data[0] = 0x03;
    newchar.inventory[3].item.data[5] = 0x04;
    newchar.inventory[3].item.itemid = 0x00010003;

    /* Clear the techniques currently available to the user. */
    memset(newchar.techniques, 0xFF, 20);

    /* Forces start out with Monofluids and Foie level 1. */
    if((ch->_class == CLASS_FONEWM) || (ch->_class == CLASS_FONEWEARL) ||
       (ch->_class == CLASS_FOMARL) || (ch->_class == CLASS_FOMAR)) {
        /* Give forces their Foie level 1. */
        newchar.techniques[0] = 0x00;

        /* Give forces their Monofluids. */
        newchar.inventory[4].in_use = 0x01;
        newchar.inventory[4].flags = 0x00;
        newchar.inventory[4].item.data[0] = 0x03;
        newchar.inventory[4].item.data[1] = 0x01;
        newchar.inventory[4].item.data[2] = 0x00;
        newchar.inventory[4].item.data[3] = 0x00;
        newchar.inventory[4].item.data[5] = 0x04;
        newchar.inventory[4].item.itemid = 0x00010004;
        newchar.inventoryUse = 5;
    }
    else {
        newchar.inventoryUse = 4;
    }

    /* Clear out all the other items. */
    for(i = newchar.inventoryUse; i < 30; i++) {
        newchar.inventory[i].in_use = 0x00;
        newchar.inventory[i].item.data[1] = 0xFF;
        newchar.inventory[i].item.itemid = 0xFFFFFFFF;
    }

    /* Copy the character's starting stats as they're given to us. */
    memcpy(&newchar.ATP, starting_stats + ch->_class * 14, 14);

    /* Copy over some unknown junk. */
    memcpy(newchar.unknown, base->unknown, 8);
    newchar.unknown2 = base->unknown2;

    /* Everyone starts with 300 Meseta. */
    newchar.meseta = 300;

    /* Store the character's Guild Card # string. */
    memcpy(newchar.gcString, ch->gcString, 10);

    /* Do some more copying from the client packet. */
    memcpy(newchar.unknown3, ch->unknown2, 14);
    newchar.nameColorBlue = ch->nameColorBlue;
    newchar.nameColorGreen = ch->nameColorGreen;
    newchar.nameColorRed = ch->nameColorRed;
    newchar.nameColorTransparency = ch->nameColorTransparency;
    newchar.skinID = ch->skinID;
    memcpy(newchar.unknown4, ch->unknown3, 18);
    newchar.sectionID = ch->sectionID;
    newchar._class = ch->_class;
    newchar.skinFlag = ch->skinFlag;
    memcpy(newchar.unknown5, ch->unknown4, 5);
    newchar.costume = ch->costume;
    newchar.skin = ch->skin;
    newchar.face = ch->face;
    newchar.head = ch->head;
    newchar.hair = ch->hair;
    newchar.hairColorRed = ch->hairColorRed;
    newchar.hairColorBlue = ch->hairColorBlue;
    newchar.hairColorGreen = ch->hairColorGreen;
    newchar.proportionX = ch->proportionX;
    newchar.proportionY = ch->proportionY;

    /* Sanitize and copy the name over. */
    n = (unsigned short *)&ch->name[4];
    for(i = 0; i < 10; i++) {
        if(*n == 0x0000)
            break;
        if((*n == 0x0009) || (*n == 0x000A))
            *n = 0x0020;
        n++;
    }

    memcpy(newchar.name, ch->name, 24);
    memcpy(newchar.name2, ch->name, 24);

    /* Back to copying stuff... */
    memcpy(newchar.unknown6, ch->unknown5, 4);
    newchar.guildCard = gc;
    newchar.sectionID2 = ch->sectionID;
    newchar._class2 = ch->_class;

    /* Copy some stuff out of the base character. */
    memcpy(newchar.keyConfig, base->keyConfig, 232);
    memcpy(newchar.unknown7, base->unknown7, 16);
    memcpy(newchar.options, base->options, 4);
    memcpy(newchar.unknown8, base->unknown8, 520);
    newchar.bankUse = base->bankUse;
    newchar.bankMeseta = base->bankMeseta;
    memcpy(newchar.bankInventory, base->bankInventory, 24 * 200);
    memcpy(newchar.unknown9, base->unknown9, 232);
    memcpy(newchar.unknown10, base->unknown10, 4);
    memcpy(newchar.symbol_chats, base->symbol_chats, 1248);
    memcpy(newchar.shortcuts, base->shortcuts, 2624);
    memcpy(newchar.unknown11, base->unknown11, 344);
    memcpy(newchar.GCBoard, base->GCBoard, 172);
    memcpy(newchar.unknown12, base->unknown12, 200);
    memcpy(newchar.challengeData, base->challengeData, 320);
    memcpy(newchar.unknown13, base->unknown13, 172);
    memcpy(newchar.unknown14, base->unknown14, 276);
    memcpy(newchar.keyConfigGlobal, base->keyConfigGlobal, 364);
    memcpy(newchar.joyConfigGlobal, base->joyConfigGlobal, 56);
    memcpy(&newchar.guildCard2, &base->guildCard2, 2108);

    /* Set up the two reserved values... */
    newchar.reserved1 = 0x01;
    newchar.reserved2 = 0x01;

    /* Delete any existing character at that position. */
    sprintf(query, "DELETE FROM character_data WHERE guildcard='%u' AND "
            "slot='%d'", gc, slot);
    sylverant_db_query(conn, query);

    /* Insert the new character at the given position. */
    sprintf(query, "INSERT INTO character_data (guildcard, slot, data, header) "
            "VALUES ('%u', '%d', '", gc, slot);
    sylverant_db_escape_str(conn, query + strlen(query), (char *)&newchar,
                            sizeof(sylverant_character_t));
    strcat(query, "', '");
    sylverant_db_escape_str(conn, query + strlen(query), ((char *)ch) + 0x10,
                            0x78);
    strcat(query, "')");

    if(sylverant_db_query(conn, query)) {
        return -1;
    }

    return 0;
}
