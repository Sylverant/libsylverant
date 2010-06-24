/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009 Lawrence Sebald

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

#include "sylverant/accounts.h"

int sylverant_acc_by_name(sylverant_dbconn_t *conn, const char *username,
                          sylverant_account_t *rv) {
    char query[256];
    int r = 0;
    void *res;
    long long row_ct;
    char **row;
    unsigned long *lengths;
    int acc_id = -1, gc = -1;

    if(!conn || !username) {
        return -42;
    }

    /* Sanity check the username. */
    if(strlen(username) > 16) {
        return -1;
    }

    r = snprintf(query, 256, "SELECT * FROM account_data WHERE username='%s'",
                 username);

    if(r == -1 || r >= 256) {
        return -1;
    }

    /* Query the database for the user. */
    if(sylverant_db_query(conn, query)) {
        return -2;
    }

    res = sylverant_db_result_store(conn);
    row_ct = sylverant_db_result_rows(res);

    if(row_ct && rv) {
        row = sylverant_db_result_fetch(res);
        lengths = sylverant_db_result_lengths(res);

        acc_id = atoi(row[0]);
        strcpy(rv->username, row[1]);
        strcpy(rv->passwd_hash, row[2]);
        strcpy(rv->email, row[3]);
        rv->regtime = strtoul(row[4], NULL, 10);
        strcpy(rv->lastip, row[5]);
        memcpy(rv->lasthwinfo, row[6], lengths[6]);
        rv->isgm = atoi(row[7]);
        rv->isbanned = atoi(row[8]);
        rv->islogged = atoi(row[9]);
        rv->isactive = atoi(row[10]);
        rv->teamid = atoi(row[11]);
        rv->privlevel = atoi(row[12]);
        memcpy(rv->lastchar, row[13], lengths[13]);
        rv->dressflag = atoi(row[14]);
        strcpy(rv->lastship, row[15]);
        rv->lastblock = atoi(row[16]);
    }

    sylverant_db_result_free(res);

    if(acc_id == -1) {
        return 1;
    }

    /* Find the guildcard number */
    r = snprintf(query, 256, "SELECT guildcard FROM guildcards WHERE "
                 "account_id='%d'", acc_id);

    if(r == -1 || r >= 256) {
        return -1;
    }

    /* Query the database for the user. */
    if(sylverant_db_query(conn, query)) {
        return -2;
    }

    res = sylverant_db_result_store(conn);

    if(row_ct) {
        row = sylverant_db_result_fetch(res);

        if(row) {
            gc = atoi(row[0]);

            if(rv) {
                rv->guildcard = gc;
            }
        }

        sylverant_db_result_free(res);
    }

    return gc == -1 ? 1 : 0;
}
