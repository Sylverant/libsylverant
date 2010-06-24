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
#include <string.h>
#include <expat.h>

#include "sylverant/config.h"

#define BUF_SIZE 8192

void dbcfg_start_hnd(void *d, const XML_Char *name, const XML_Char **attrs) {
    int i;
    sylverant_dbconfig_t *cfg = (sylverant_dbconfig_t *)d;

    if(!strcmp(name, "database"))   {
        for(i = 0; attrs[i]; i += 2)    {
            if(!strcmp(attrs[i], "type"))   {
                strncpy(cfg->type, attrs[i + 1], 255);
                cfg->type[255] = '\0';
            }
            else if(!strcmp(attrs[i], "host"))  {
                strncpy(cfg->host, attrs[i + 1], 255);
                cfg->host[255] = '\0';
            }
            else if(!strcmp(attrs[i], "user"))  {
                strncpy(cfg->user, attrs[i + 1], 255);
                cfg->user[255] = '\0';
            }
            else if(!strcmp(attrs[i], "pass"))  {
                strncpy(cfg->pass, attrs[i + 1], 255);
                cfg->pass[255] = '\0';
            }
            else if(!strcmp(attrs[i], "db"))    {
                strncpy(cfg->db, attrs[i + 1], 255);
                cfg->db[255] = '\0';
            }
            else if(!strcmp(attrs[i], "port"))  {
                cfg->port = atoi(attrs[i + 1]);
            }
        }
    }
}

void dbcfg_end_hnd(void *d, const XML_Char *name)   {
}

int sylverant_read_dbconfig(sylverant_dbconfig_t *cfg) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;

    /* Open the configuration file for reading. */
    fp = fopen(SYLVERANT_CFG, "r");

    if(!fp) {
        return -1;
    }

    /* Create the XML parser object. */
    p = XML_ParserCreate(NULL);

    if(!p)  {
        fclose(fp);
        return -2;
    }

    XML_SetElementHandler(p, &dbcfg_start_hnd, &dbcfg_end_hnd);

    /* Set up the user data so we can store the configuration. */
    XML_SetUserData(p, cfg);

    for(;;) {
        /* Grab the buffer to read into. */
        buf = XML_GetBuffer(p, BUF_SIZE);

        if(!buf)    {
            XML_ParserFree(p);
            return -2;
        }

        /* Read in from the file. */
        bytes = fread(buf, 1, BUF_SIZE, fp);

        if(bytes < 0)   {
            XML_ParserFree(p);
            return -2;
        }

        /* Parse the bit we read in. */
        if(!XML_ParseBuffer(p, bytes, !bytes))  {
            XML_ParserFree(p);
            return -3;
        }

        if(!bytes)  {
            break;
        }
    }

    XML_ParserFree(p);
    return 0;
}
