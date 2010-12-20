/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010 Lawrence Sebald

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
#include <expat.h>

#include <arpa/inet.h>

#include "sylverant/config.h"

#define BUF_SIZE 8192

static int in_proxy = 0;

static void cfg_start_hnd(void *d, const XML_Char *name,
                          const XML_Char **attrs) {
    int i;
    unsigned long tmp;
    sylverant_proxycfg_t *cfg = (sylverant_proxycfg_t *)d;

    if(!strcmp(name, "proxy")) {
        in_proxy = 1;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "name")) {
                strncpy(cfg->name, attrs[i + 1], 255);
                cfg->name[255] = '\0';
            }
            else if(!strcmp(attrs[i], "key")) {
                strncpy(cfg->key_file, attrs[i + 1], 255);
                cfg->key_file[255] = '\0';
            }
            else if(!strcmp(attrs[i], "gmonly")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    cfg->shipgate_flags |= SHIPGATE_FLAG_GMONLY;
                }
            }
        }
    }
    else if(!strcmp(name, "shipgate") && in_proxy) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "ip")) {
                cfg->shipgate_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "port")) {
                cfg->shipgate_port = (uint16_t)atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "net") && in_proxy) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "ip")) {
                cfg->proxy_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "port")) {
                cfg->base_port = (uint16_t)atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "for") && in_proxy) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "server")) {
                cfg->server_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "ignore") && in_proxy) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "packet")) {
                tmp = strtoul(attrs[i + 1], NULL, 0);

                if(tmp > 0 && tmp < 256) {
                    cfg->packets[tmp] = 1;
                }
            }
        }
    }
}

static void cfg_end_hnd(void *d, const XML_Char *name) {
    if(!strcmp(name, "proxy")) {
        in_proxy = 0;
    }
}

int sylverant_read_proxy_config(sylverant_proxycfg_t *cfg) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;

    /* Clear out the config. */
    memset(cfg, 0, sizeof(sylverant_proxycfg_t));

    /* Open the configuration file for reading. */
    fp = fopen(sylverant_proxy_cfg, "r");

    if(!fp) {
        return -1;
    }

    /* Create the XML parser object. */
    p = XML_ParserCreate(NULL);

    if(!p)  {
        fclose(fp);
        return -2;
    }

    XML_SetElementHandler(p, &cfg_start_hnd, &cfg_end_hnd);

    /* Set up the user data so we can store the configuration. */
    XML_SetUserData(p, cfg);

    in_proxy = 0;

    for(;;) {
        /* Grab the buffer to read into. */
        buf = XML_GetBuffer(p, BUF_SIZE);

        if(!buf)    {
            XML_ParserFree(p);
            fclose(fp);
            return -2;
        }

        /* Read in from the file. */
        bytes = fread(buf, 1, BUF_SIZE, fp);

        if(bytes < 0)   {
            XML_ParserFree(p);
            fclose(fp);
            return -2;
        }

        /* Parse the bit we read in. */
        if(!XML_ParseBuffer(p, bytes, !bytes))  {
            XML_ParserFree(p);
            fclose(fp);
            return -3;
        }

        if(!bytes)  {
            break;
        }
    }

    XML_ParserFree(p);
    fclose(fp);

    return 0;
}
