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

#ifndef SYLVERANT__ACCOUNTS_H
#define SYLVERANT__ACCOUNTS_H

#include "sylverant/database.h"

typedef struct sylverant_account    {
    char username[18];
    char passwd_hash[33];
    char email[255];
    unsigned int regtime;
    char lastip[16];
    unsigned char lasthwinfo[256];
    int guildcard;
    int isgm;
    int isbanned;
    int islogged;
    int isactive;
    int teamid;
    int privlevel;
    unsigned char lastchar[256];
    int dressflag;
    char lastship[12];
    int lastblock;
} sylverant_account_t;
    

extern int sylverant_acc_by_name(sylverant_dbconn_t *conn, const char *username,
                                 sylverant_account_t *rv);
extern int sylverant_acc_by_gc(sylverant_dbconn_t *conn, unsigned long long gc,
                               sylverant_account_t *rv);

extern unsigned long long sylverant_acc_count(sylverant_dbconn_t *conn);

extern int sylverant_acc_add(sylverant_dbconn_t *conn, const char *username,
                             const char *passwd, const char *email, int isgm,
                             int *guildcard);

#endif /* !SYLVERANT__ACCOUNTS_H */
