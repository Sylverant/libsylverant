/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011 Lawrence Sebald

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

#ifndef SYLVERANT__CONFIG_H
#define SYLVERANT__CONFIG_H

#include <stdint.h>

/* Various directories and files */
extern const char sylverant_directory[];
extern const char sylverant_cfg[];
extern const char sylverant_ship_cfg[];

/* Values for the shipgate flags portion of ships/proxies. */
#define SHIPGATE_FLAG_GMONLY    0x00000001
#define SHIPGATE_FLAG_NOV1      0x00000010
#define SHIPGATE_FLAG_NOV2      0x00000020
#define SHIPGATE_FLAG_NOPC      0x00000040
#define SHIPGATE_FLAG_NOEP12    0x00000080
#define SHIPGATE_FLAG_NOEP3     0x00000100

typedef struct sylverant_dbconfig {
    char *type;
    char *host;
    char *user;
    char *pass;
    char *db;
    uint16_t port;
} sylverant_dbconfig_t;

typedef struct sylverant_config {
    sylverant_dbconfig_t dbcfg;
    uint32_t server_ip;
    uint8_t server_ip6[16];
    uint16_t server_port;
    char *quests_dir;
    char *limits_file;
} sylverant_config_t;

typedef struct sylverant_shipcfg {
    uint32_t shipgate_ip;
    uint8_t shipgate_ip6[16];
    uint16_t shipgate_port;

    char *name;
    char *key_file;
    char *gm_file;
    char *limits_file;
    char *motd_file;
    char **info_files;
    char **info_files_desc;
    char *quests_file;
    char *quests_dir;
    char *bans_file;
    char *scripts_file;

    uint32_t ship_ip;
    uint8_t ship_ip6[16];
    uint32_t shipgate_flags;

    uint16_t base_port;
    uint16_t menu_code;

    int blocks;
    int info_file_count;

    int game_event;
    int lobby_event;
} sylverant_ship_t;

/* Read the configuration for the login server, shipgate, and patch server. */
extern int sylverant_read_config(sylverant_config_t **cfg);

/* Clean up a configuration structure. */
extern void sylverant_free_config(sylverant_config_t *cfg);

/* Read the ship configuration data. You are responsible for calling the
   function to clean up the configuration. */
extern int sylverant_read_ship_config(const char *f, sylverant_ship_t **cfg);

/* Clean up a ship configuration structure. */
extern void sylverant_free_ship_config(sylverant_ship_t *cfg);

#endif /* !SYLVERANT__CONFIG_H */
