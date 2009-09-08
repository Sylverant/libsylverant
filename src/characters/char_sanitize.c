/*
    This file is part of Sylverant PSO Server.

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

/* All of this code is adapted from the Tethealla Login Server's
   AckCharacter_Creation function. */
void sylverant_char_sanitize(sylverant_mini_char_t *ch) {
    unsigned short maxFace, maxHair, maxHairColorRed, maxHairColorBlue,
        maxHairColorGreen, maxCostume, maxSkin, maxHead;
    int i;

    ch->name[0] = 0x09;    // Filter colored names
    ch->name[1] = 0x00;
    ch->name[2] = 0x45;
    ch->name[3] = 0x00;
    ch->name[22] = 0;      // Truncate names too long
    ch->name[23] = 0;

    if((ch->_class == CLASS_HUMAR) || (ch->_class == CLASS_HUNEWEARL) ||
       (ch->_class == CLASS_RAMAR) || (ch->_class == CLASS_RAMARL) ||
       (ch->_class == CLASS_FOMARL) || (ch->_class == CLASS_FONEWM) ||
       (ch->_class == CLASS_FONEWEARL) || (ch->_class == CLASS_FOMAR)) {
        maxFace = 0x05;
        maxHair = 0x0A;
        maxHairColorRed = 0xFF;
        maxHairColorBlue = 0xFF;
        maxHairColorGreen = 0xFF;
        maxCostume = 0x11;
        maxSkin = 0x03;
        maxHead = 0x00;
    }
    else {
        maxFace = 0x00;
        maxHair = 0x00;
        maxHairColorRed = 0x00;
        maxHairColorBlue = 0x00;
        maxHairColorGreen = 0x00;
        maxCostume = 0x00;
        maxSkin = 0x18;
        maxHead = 0x04;
    }

    if(ch->skinID > 0x06)
        ch->skinID = 0x00;

    ch->nameColorTransparency = 0xFF;

    if(ch->sectionID > 0x09)
        ch->sectionID = 0x00;
    if(ch->proportionX > 0x3F800000)
        ch->proportionX = 0x3F800000;
    if(ch->proportionY > 0x3F800000)
        ch->proportionY = 0x3F800000;
    if(ch->face > maxFace)
        ch->face = 0x00;
    if(ch->hair > maxHair)
        ch->hair = 0x00;
    if(ch->hairColorRed > maxHairColorRed)
        ch->hairColorRed = 0x00;
    if(ch->hairColorBlue > maxHairColorBlue)
        ch->hairColorBlue = 0x00;
    if(ch->hairColorGreen > maxHairColorGreen)
        ch->hairColorGreen = 0x00;
    if(ch->costume > maxCostume)
        ch->costume = 0x00;
    if(ch->skin > maxSkin)
        ch->skin = 0x00;
    if(ch->head > maxHead)
        ch->head = 0x00;

    for(i = 0; i < 8; i++)
        ch->unknown5[i] = 0x00;

    ch->playTime = 0;
}
