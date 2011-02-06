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
    char type[256];
    char host[256];
    char user[256];
    char pass[256];
    char db[256];
    unsigned int port;
} sylverant_dbconfig_t;

typedef struct sylverant_config {
    sylverant_dbconfig_t dbcfg;
    uint32_t server_ip;
    uint32_t override_ip;
    uint32_t netmask;
    int override_on;
    uint16_t server_port;
    char *welcome_message;
    char quests_dir[256];
    char limits_file[256];

    struct {
        int maxconn;
        int throttle;
    } patch;

    struct {
        int maxconn;
    } login;

    struct {
        int maxships;
    } shipgate;

    struct {
        int hildebear;
        int rappy;
        int lilly;
        int slime;
        int merissa;
        int pazuzu;
        int dorphon;
        int kondrieu;
    } rare_monsters;

    struct {
        uint32_t ggm;
        uint32_t lgm;
        uint32_t user;
    } colors;
} sylverant_config_t;

typedef struct sylverant_ship {
    char name[256];
    char key_file[256];
    char gm_file[256];
    char limits_file[256];
    char motd_file[256];
    char **info_files;
    char **info_files_desc;
    char *quests_file;
    char *quests_dir;
    char *bans_file;

    uint32_t ship_ip;
    uint32_t shipgate_flags;

    uint16_t base_port;
    uint16_t menu_code;

    int blocks;
    int info_file_count;

    struct {
        float weapon;
        float armor;
        float mag;
        float tool;
        float meseta;
    } drops;

    int game_event;
    int lobby_event;
    float exp_rate;
} sylverant_ship_t;

typedef struct sylverant_shipcfg {
    uint32_t shipgate_ip;
    uint16_t shipgate_port;

    int ship_count;
    sylverant_ship_t ships[0];
} sylverant_shipcfg_t;

/* For when you only want database configuration. */
extern int sylverant_read_dbconfig(sylverant_dbconfig_t *cfg);

/* For when you want everything. */
extern int sylverant_read_config(sylverant_config_t *cfg);

/* Read the ship configuration data. You are responsible for calling the
   function to clean up the configuration. */
extern int sylverant_read_ship_config(const char *f, sylverant_shipcfg_t **cfg);

/* Clean up a ship configuration structure. */
extern int sylverant_free_ship_config(sylverant_shipcfg_t *cfg);

#endif /* !SYLVERANT__CONFIG_H */
