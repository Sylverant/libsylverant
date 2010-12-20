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
#include <ctype.h>

#include <arpa/inet.h>

#include "sylverant/config.h"

#define BUF_SIZE 8192

static int in_ship = 0;

static void cfg_start_hnd(void *d, const XML_Char *name,
                          const XML_Char **attrs) {
    int i, tmp3;
    sylverant_shipcfg_t *cfg = *(sylverant_shipcfg_t **)d;
    void *tmp, **tmp2;
    char str[256];
    sylverant_ship_t *cur = NULL;

    if(cfg->ship_count) {
        cur = &cfg->ships[cfg->ship_count - 1];
    }

    if(!strcmp(name, "ship")) {
        tmp = realloc(cfg, sizeof(sylverant_shipcfg_t) +
                      ((cfg->ship_count + 1) * sizeof(sylverant_ship_t)));

        /* Don't read the ship if we can't allocate the memory properly. */
        if(tmp) {
            cfg = (sylverant_shipcfg_t *)tmp;

            /* Clear the new ship */
            memset(cfg->ships + cfg->ship_count, 0, sizeof(sylverant_ship_t));
            ++cfg->ship_count;

            tmp2 = (void **)d;
            *tmp2 = cfg;
            in_ship = 1;

            for(i = 0; attrs[i]; i += 2) {
                if(!strcmp(attrs[i], "name")) {
                    strncpy(cfg->ships[cfg->ship_count - 1].name,
                            attrs[i + 1], 255);
                    cfg->ships[cfg->ship_count - 1].name[255] = '\0';
                }
                else if(!strcmp(attrs[i], "blocks")) {
                    cfg->ships[cfg->ship_count - 1].blocks = atoi(attrs[i + 1]);
                }
                else if(!strcmp(attrs[i], "key")) {
                    strncpy(cfg->ships[cfg->ship_count - 1].key_file,
                            attrs[i + 1], 255);
                    cfg->ships[cfg->ship_count - 1].key_file[255] = '\0';
                }
                else if(!strcmp(attrs[i], "gms")) {
                    strncpy(cfg->ships[cfg->ship_count - 1].gm_file,
                            attrs[i + 1], 255);
                    cfg->ships[cfg->ship_count - 1].gm_file[255] = '\0';
                }
                else if(!strcmp(attrs[i], "menu")) {
                    if(strlen(attrs[i + 1]) == 2 && isalpha(attrs[i + 1][0]) &&
                       isalpha(attrs[i + 1][1])) {
                        cfg->ships[cfg->ship_count - 1].menu_code =
                            ((uint16_t)attrs[i + 1][0]) |
                            (((uint16_t)(attrs[i + 1][1])) << 8);
                    }
                }
            }
        }
    }
    else if(!strcmp(name, "shipgate")) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "ip")) {
                cfg->shipgate_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "port")) {
                cfg->shipgate_port = (uint16_t)atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "net") && in_ship) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "ip")) {
                cur->ship_ip = (uint32_t)inet_addr(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "port")) {
                cur->base_port = (uint16_t)atoi(attrs[i + 1]);
            }
        }
    }    
    else if(!strcmp(name, "drops") && in_ship) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "weapon")) {
                cur->drops.weapon = strtof(attrs[i + 1], NULL);
            }
            else if(!strcmp(attrs[i], "armor")) {
                cur->drops.armor = strtof(attrs[i + 1], NULL);
            }
            else if(!strcmp(attrs[i], "mag")) {
                cur->drops.mag = strtof(attrs[i + 1], NULL);
            }
            else if(!strcmp(attrs[i], "tool")) {
                cur->drops.tool = strtof(attrs[i + 1], NULL);
            }
            else if(!strcmp(attrs[i], "meseta")) {
                cur->drops.meseta = strtof(attrs[i + 1], NULL);
            }
        }
    }
    else if(!strcmp(name, "event") && in_ship) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "num")) {
                cur->event = atoi(attrs[i + 1]);
            }
        }
    }
    else if(!strcmp(name, "exp") && in_ship) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "rate")) {
                cur->exp_rate = strtof(attrs[i + 1], NULL);
            }
        }
    }
    else if(!strcmp(name, "quests") && in_ship) {
        /* Only pay attention to the first <quests> in a ship. */
        if(!cur->quests_file) {
            for(i = 0; attrs[i]; i += 2) {
                if(!strcmp(attrs[i], "file")) {
                    cur->quests_file = (char *)malloc(strlen(attrs[i + 1]) + 1);
                    strcpy(cur->quests_file, attrs[i + 1]);
                }
            }
        }
    }
    else if(!strcmp(name, "info") && in_ship) {
        tmp3 = cur->info_file_count;

        /* Allocate space for the new filename. */
        tmp = realloc(cur->info_files, (tmp3 + 1) * sizeof(char *));
        if(!tmp) {
            return;
        }

        cur->info_files = (char **)tmp;

        /* Allocate space for the new description. */
        tmp = realloc(cur->info_files_desc, (tmp3 + 1) * sizeof(char *));
        if(!tmp) {
            return;
        }

        cur->info_files_desc = (char **)tmp;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "file")) {
                cur->info_files[tmp3] =
                    (char *)malloc(strlen(attrs[i + 1]) + 1);

                if(!cur->info_files[tmp3]) {
                    return;
                }

                strcpy(cur->info_files[tmp3], attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "desc")) {
                cur->info_files_desc[tmp3] =
                    (char *)malloc(strlen(attrs[i + 1]) + 1);

                if(!cur->info_files_desc[tmp3]) {
                    return;
                }

                strcpy(cur->info_files_desc[tmp3], attrs[i + 1]);
            }
        }

        ++cur->info_file_count;
    }
    else if(!strcmp(name, "limits") && in_ship) {
        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "file")) {
                strncpy(cur->limits_file, attrs[i + 1], 255);
                cur->limits_file[255] = '\0';
            }
        }
    }
    else if(!strcmp(name, "motd") && in_ship) {
        if(!attrs || !attrs[0] || !attrs[1] || attrs[2]) {
            return;
        }

        if(!strcmp(attrs[0], "file")) {
            strncpy(cur->motd_file, attrs[1], 255);
            cur->motd_file[255] = '\0';
        }
    }
}

static void cfg_end_hnd(void *d, const XML_Char *name) {
    if(!strcmp(name, "ship")) {
        in_ship = 0;
    }
}

int sylverant_read_ship_config(const char *f, sylverant_shipcfg_t **cfg) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;
    sylverant_shipcfg_t *rv;

    /* Allocate space for the base of the config. */
    rv = (sylverant_shipcfg_t *)malloc(sizeof(sylverant_shipcfg_t));

    if(!rv) {
        *cfg = NULL;
        return -4;
    }

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_shipcfg_t));

    /* Open the configuration file for reading. */
    if(!f) {
        fp = fopen(sylverant_ship_cfg, "r");
    }
    else {
        fp = fopen(f, "r");
    }

    if(!fp) {
        free(rv);
        *cfg = NULL;
        return -1;
    }

    /* Create the XML parser object. */
    p = XML_ParserCreate(NULL);

    if(!p)  {
        fclose(fp);
        free(rv);
        *cfg = NULL;
        return -2;
    }

    XML_SetElementHandler(p, &cfg_start_hnd, &cfg_end_hnd);

    /* Set up the user data so we can store the configuration. */
    XML_SetUserData(p, &rv);

    in_ship = 0;

    for(;;) {
        /* Grab the buffer to read into. */
        buf = XML_GetBuffer(p, BUF_SIZE);

        if(!buf)    {
            XML_ParserFree(p);
            free(rv);
            fclose(fp);
            *cfg = NULL;
            return -2;
        }

        /* Read in from the file. */
        bytes = fread(buf, 1, BUF_SIZE, fp);

        if(bytes < 0)   {
            XML_ParserFree(p);
            free(rv);
            fclose(fp);
            *cfg = NULL;
            return -2;
        }

        /* Parse the bit we read in. */
        if(!XML_ParseBuffer(p, bytes, !bytes))  {
            XML_ParserFree(p);
            free(rv);
            fclose(fp);
            *cfg = NULL;
            return -3;
        }

        if(!bytes)  {
            break;
        }
    }

    XML_ParserFree(p);
    fclose(fp);
    *cfg = rv;

    return 0;
}

int sylverant_free_ship_config(sylverant_shipcfg_t *cfg) {
    int i, j;

    /* Make sure we actually have a valid configuration pointer. */
    if(cfg) {
        /* Look through each ship to clean up its info files. */
        for(i = 0; i < cfg->ship_count; ++i) {
            if(cfg->ships[i].info_files) {
                for(j = 0; j < cfg->ships[i].info_file_count; ++j) {
                    free(cfg->ships[i].info_files[j]);
                    free(cfg->ships[i].info_files_desc[j]);
                }

                free(cfg->ships[i].info_files);
                free(cfg->ships[i].info_files_desc);
            }

            free(cfg->ships[i].quests_file);
        }

        /* Clean up the base structure. */
        free(cfg);
    }
}
