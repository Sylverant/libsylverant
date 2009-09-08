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
#include <time.h>

#include "sylverant/accounts.h"

int sylverant_acc_add(sylverant_dbconn_t *conn, const char *username,
                      const char *passwd, const char *email, int isgm,
                      int *guildcard) {
    char query[1024];
    time_t t;
    int rv;
    uint32_t acc_id;

    if(!conn) {
        return -42;
    }

    /* Salt the password with the current time. */
    t = time(NULL);

    /* Build the query up. */
    rv = snprintf(query, 1024, "INSERT INTO account_data(username, "
                  "password, email, regtime, isgm, isactive) VALUES('%s', "
                  "MD5('%s_%lld_salt'), '%s', '%lld', '%d', 1)", username,
                  passwd, (long long int)t, email, (long long int)t, isgm);

    if(rv == -1 || rv >= 1024) {
        return -1;
    }

    /* Do it. */
    if(sylverant_db_query(conn, query)) {
        return -2;
    }

    acc_id = (uint32_t)sylverant_db_insert_id(conn);

    /* Fetch the user's guildcard by adding to the guildcards table. */
    snprintf(query, 1024, "INSERT INTO guildcards(account_id) VALUES('%u')",
             acc_id);

    if(rv == -1 || rv >= 1024) {
        return -1;
    }

    if(sylverant_db_query(conn, query)) {
        return -3;
    }

    /* Grab the user's new guildcard number. */
    *guildcard = (int)sylverant_db_insert_id(conn);
    
    return 0;

}
