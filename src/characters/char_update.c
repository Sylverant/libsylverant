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
#include <string.h>
#include "sylverant/characters.h"

/* Maximum length of a SQL query to put a character in the DB. */
#define SZ sizeof(sylverant_character_t) * 2 + \
    sizeof(sylverant_mini_char_t) * 2 + 256

int sylverant_char_update(sylverant_dbconn_t *conn, uint32_t gc, int slot,
                          sylverant_mini_char_t *ch) {
    sylverant_character_t orig;
    int rv;
    char query[SZ];

    if((rv = sylverant_char_fetch(conn, gc, slot, &orig))) {
        return rv;
    }

    /* Copy over the new data. */
    memcpy(orig.gcString, ch->gcString, 0x68);
    memcpy(ch->unknown5, orig.unknown6, 8);

    /* Update the character at the given position. */
    sprintf(query, "UPDATE character_data SET data='");
    sylverant_db_escape_str(conn, query + strlen(query), (char *)&orig,
                            sizeof(sylverant_character_t));
    strcat(query, "', header='");
    sylverant_db_escape_str(conn, query + strlen(query), ((char *)ch) + 0x10,
                            0x78);
    sprintf(query + strlen(query), "' WHERE guildcard='%u' AND slot='%d'", gc,
           slot);

    if(sylverant_db_query(conn, query)) {
        return -1;
    }

    return 0;
}
