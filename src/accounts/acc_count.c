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

#include <string.h>
#include <stdlib.h>

#include "sylverant/accounts.h"

unsigned long long sylverant_acc_count(sylverant_dbconn_t *conn) {
    char query[256];
    unsigned long long rv = 0;

    if(!conn) {
        return 0xFEEDFACEDEADBEEFULL;
    }

    strcpy(query, "SELECT COUNT(*) FROM account_data");

    /* Query the database. */
    if(!sylverant_db_query(conn, query)) {
        void *res = sylverant_db_result_store(conn);
        char **row;

        row = sylverant_db_result_fetch(res);

        rv = strtoull(row[0], NULL, 10);

        sylverant_db_result_free(res); 
    }

    return rv;
}
