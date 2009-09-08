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
#include <stdlib.h>
#include <string.h>

#include "sylverant/accounts.h"

int sylverant_acc_by_gc(sylverant_dbconn_t *conn, unsigned long long gc,
                        sylverant_account_t *rv) {
    char query[256];
    int r = 0;
    void *res;
    char **row;
    long long row_ct;
    unsigned long *lengths;
    int acc_id;

    if(!conn) {
        return -42;
    }

    /* Fetch the user's account id first. */
    r = snprintf(query, 256, "SELECT account_id FROM guildcards WHERE "
                 "guildcard='%lld'", gc);

    if(r >= 256 || r == -1) {
        return -1;
    }

    /* Look up the account id. */
    if(sylverant_db_query(conn, query)) {
        return -2;
    }

    res = sylverant_db_result_store(conn);
    row_ct = sylverant_db_result_rows(res);

    if(row_ct != 1) {
        return -3;
    }

    row = sylverant_db_result_fetch(res);
    acc_id = atoi(row[0]);
    sylverant_db_result_free(res);

    /* Now that we have the account id, look up the user. */
    r = snprintf(query, 256, "SELECT * FROM account_data WHERE account_id='%d'",
                 acc_id);

    if(r >= 256 || r == -1) {
        return -1;
    }

    /* Query the database for the user. */
    if(!sylverant_db_query(conn, query)) {
        res = sylverant_db_result_store(conn);
        row_ct = sylverant_db_result_rows(res);

        r = !!row_ct;

        if(r && rv) {
            row = sylverant_db_result_fetch(res);
            lengths = sylverant_db_result_lengths(res);

            strcpy(rv->username, row[1]);
            strcpy(rv->passwd_hash, row[2]);
            strcpy(rv->email, row[3]);
            rv->regtime = strtoul(row[4], NULL, 10);
            strcpy(rv->lastip, row[5]);
            memcpy(rv->lasthwinfo, row[6], lengths[6]);
            rv->guildcard = gc;
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
    }
    else {
        return -2;
    }

    return r;
}
