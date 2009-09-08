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
#include <expat.h>

#include <arpa/inet.h>

#include "sylverant/config.h"

#define BUF_SIZE 8192

static void cfg_start_hnd(void *d, const XML_Char *name,
                          const XML_Char **attrs) {
    int i;
    sylverant_config_t *cfg = (sylverant_config_t *)d;

    if(!strcmp(name, "database")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "type")) {
                strncpy(cfg->dbcfg.type, attrs[i + 1], 255);
                cfg->dbcfg.type[255] = '\0';
            }
            else if(!strcmp(attrs[i], "host")) {
                strncpy(cfg->dbcfg.host, attrs[i + 1], 255);
                cfg->dbcfg.host[255] = '\0';
            }
            else if(!strcmp(attrs[i], "user")) {
                strncpy(cfg->dbcfg.user, attrs[i + 1], 255);
                cfg->dbcfg.user[255] = '\0';
            }
            else if(!strcmp(attrs[i], "pass")) {
                strncpy(cfg->dbcfg.pass, attrs[i + 1], 255);
                cfg->dbcfg.pass[255] = '\0';
            }
            else if(!strcmp(attrs[i], "db")) {
                strncpy(cfg->dbcfg.db, attrs[i + 1], 255);
                cfg->dbcfg.db[255] = '\0';
            }
            else if(!strcmp(attrs[i], "port")) {
                cfg->dbcfg.port = atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "server")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "addr")) {
                cfg->server_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "port")) {
                cfg->server_port = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "override")) {
                cfg->override_ip = (uint32_t)inet_addr(attrs[i + 1]);
                cfg->override_on = 1;
            }
            else if(!strcmp(attrs[i], "netmask")) {
                cfg->netmask = (uint32_t)inet_addr(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "patch")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "connections")) {
                cfg->patch.maxconn = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "throttle")) {
                cfg->patch.throttle = atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "colors")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "globalGM")) {
                cfg->colors.ggm = (uint32_t)strtoul(attrs[i + 1], NULL, 0);
            }
            else if(!strcmp(attrs[i], "localGM")) {
                cfg->colors.lgm = (uint32_t)strtoul(attrs[i + 1], NULL, 0);
            }
            else if(!strcmp(attrs[i], "user")) {
                cfg->colors.user = (uint32_t)strtoul(attrs[i + 1], NULL, 0);
            }
        }
    }
    else if(!strcmp(name, "login")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "maxconn")) {
                cfg->login.maxconn = atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "shipgate")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "maxships")) {
                cfg->shipgate.maxships = atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "raremonsters")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "hildebear")) {
                cfg->rare_monsters.hildebear = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "rappy")) {
                cfg->rare_monsters.rappy = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "lilly")) {
                cfg->rare_monsters.lilly = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "slime")) {
                cfg->rare_monsters.slime = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "merissa")) {
                cfg->rare_monsters.merissa = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "pazuzu")) {
                cfg->rare_monsters.pazuzu = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "dorphon")) {
                cfg->rare_monsters.dorphon = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "kondrieu")) {
                cfg->rare_monsters.kondrieu = atoi(attrs[i + 1]);
            }
        }
    }
}

static void cfg_end_hnd(void *d, const XML_Char *name) {
}

int sylverant_read_config(sylverant_config_t *cfg) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;

    /* Clear out the config. */
    memset(cfg, 0, sizeof(sylverant_config_t));

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

    XML_SetElementHandler(p, &cfg_start_hnd, &cfg_end_hnd);

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
