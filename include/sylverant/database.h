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

#ifndef SYLVERANT__DATABASE_H
#define SYLVERANT__DATABASE_H

#include "sylverant/config.h"

typedef struct sylverant_dbconn {
    void *conndata;
} sylverant_dbconn_t;

extern int sylverant_db_open(sylverant_dbconfig_t *dbcfg,
                             sylverant_dbconn_t *conn);
extern void sylverant_db_close(sylverant_dbconn_t *conn);

extern int sylverant_db_query(sylverant_dbconn_t *conn, const char *str);

extern void *sylverant_db_result_store(sylverant_dbconn_t *conn);
extern void sylverant_db_result_free(void *result);

extern long long int sylverant_db_result_rows(void *result);
extern unsigned int sylverant_db_result_fields(sylverant_dbconn_t *conn);
extern char **sylverant_db_result_fetch(void *result);
unsigned long *sylverant_db_result_lengths(void *result);

extern long long int sylverant_db_insert_id(sylverant_dbconn_t *conn);
extern unsigned long sylverant_db_escape_str(sylverant_dbconn_t *conn, char *to,
                                             const char *from,
                                             unsigned long len);
extern const char *sylverant_db_error(sylverant_dbconn_t *conn);

#endif /* !SYLVERANT__DATABASE_H */
