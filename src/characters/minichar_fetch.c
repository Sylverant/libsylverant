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

int sylverant_char_fetch_mini(sylverant_dbconn_t *conn, uint32_t gc, int slot,
                              sylverant_mini_char_t *rv) {
    char query[256];
    int r;

    if(!conn) {
        return -42;
    }

    /* Make sure the slot number is valid */
    if(slot < 0 || slot > 3) {
        return -2;
    }

    /* Build the query. */
    r = snprintf(query, 256, "SELECT header FROM character_data WHERE "
                 "guildcard='%u' AND slot='%d'", gc, slot);

    if(r == -1 || r >= 256) {
        return -1;
    }

    /* Query the database. */
    if(!sylverant_db_query(conn, query)) {
        void *res = sylverant_db_result_store(conn);
        long long row_ct = sylverant_db_result_rows(res);
        char **row;

        if(row_ct && rv) {
            row = sylverant_db_result_fetch(res);

            /* Fill in character structure. */
            memcpy(&rv->level, row[0], sizeof(sylverant_mini_char_t) - 0x10);
        }

        sylverant_db_result_free(res);
        return !row_ct;
    }

    return -3;
}
