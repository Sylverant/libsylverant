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

#include "sylverant/database.h"
#include "sylverant/config.h"

#include <stdio.h>
#include <mysql.h>

int sylverant_db_open(sylverant_dbconfig_t *cfg, sylverant_dbconn_t *conn)  {
    MYSQL *mysql;
    my_bool rc = 1;

    if(!cfg || !conn)   {
        return -42;
    }

    /* Set up the MYSQL object to connect to the database. */
    mysql = mysql_init(NULL);

    if(!mysql)  {
        conn->conndata = NULL;
        return -1;
    }

    /* Attempt to connect to the MySQL server. */
    if(!mysql_real_connect(mysql, cfg->host, cfg->user, cfg->pass, NULL,
                           cfg->port, NULL, 0)) {
        mysql_close(mysql);
        conn->conndata = NULL;
        return -2;
    }

    /* Attempt to select the database requested. */
    if(mysql_select_db(mysql, cfg->db) < 0) {
        mysql_close(mysql);
        conn->conndata = NULL;
        return -3;
    }

    /* Configure to automatically reconnect if the connection is dropped. */
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &rc); 

    conn->conndata = (void *)mysql;

    return 0;
}

void sylverant_db_close(sylverant_dbconn_t *conn)   {
    if(!conn || !conn->conndata)    {
        return;
    }

    mysql_close((MYSQL *)conn->conndata);
}

int sylverant_db_query(sylverant_dbconn_t *conn, const char *str)   {
    if(!conn || !conn->conndata)    {
        return -42;
    }

    return mysql_query((MYSQL *)conn->conndata, str);
}

void *sylverant_db_result_store(sylverant_dbconn_t *conn)   {
    if(!conn || !conn->conndata)    {
        return NULL;
    }

    return (void *)mysql_store_result((MYSQL *)conn->conndata);
}

void sylverant_db_result_free(void *result) {
    if(!result) {
        return;
    }

    mysql_free_result((MYSQL_RES *)result);
}

long long int sylverant_db_result_rows(void *result)    {
    if(!result) {
        return -42;
    }

    return (long long int)mysql_num_rows((MYSQL_RES *)result);
}

unsigned int sylverant_db_result_fields(sylverant_dbconn_t *conn)   {
    if(!conn || !conn->conndata)    {
        return -42;
    }

    return mysql_field_count((MYSQL *)conn->conndata);
}

char **sylverant_db_result_fetch(void *result)  {
    if(!result) {
        return NULL;
    }

    return mysql_fetch_row((MYSQL_RES *)result);
}

unsigned long *sylverant_db_result_lengths(void *result)    {
    if(!result) {
        return NULL;
    }

    return mysql_fetch_lengths((MYSQL_RES *)result);
}

long long int sylverant_db_insert_id(sylverant_dbconn_t *conn)  {
    if(!conn || !conn->conndata)    {
        return -42;
    }

    return (long long int)mysql_insert_id((MYSQL *)conn->conndata);
}

unsigned long sylverant_db_escape_str(sylverant_dbconn_t *conn, char *to,
                                      const char *from, unsigned long len) {
    if(!conn || !conn->conndata) {
        return -42;
    }

    return mysql_real_escape_string((MYSQL *)conn->conndata, to, from, len);
}

const char *sylverant_db_error(sylverant_dbconn_t *conn) {
    if(!conn || !conn->conndata) {
        return "No Connection";
    }

    return mysql_error((MYSQL *)conn->conndata);
}
