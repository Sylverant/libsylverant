/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2010, 2011, 2012, 2013, 2016, 2018, 2019, 2020, 2021,
                  2024 Lawrence Sebald

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
#define SHIPGATE_FLAG_NOBB      0x00000200
#define SHIPGATE_FLAG_NODCNTE   0x00000400
#define SHIPGATE_FLAG_NOPSOX    0x00000800
#define SHIPGATE_FLAG_NOPCNTE   0x00001000

/* The first few (V1, V2, PC) are only valid on the ship server, whereas the
   last couple (Ep3, BB) are only valid on the login server. GC works either
   place, but only some GC versions can see info files on the ship. */
#define SYLVERANT_INFO_V1       0x00000001
#define SYLVERANT_INFO_V2       0x00000002
#define SYLVERANT_INFO_PC       0x00000004
#define SYLVERANT_INFO_GC       0x00000008
#define SYLVERANT_INFO_EP3      0x00000010
#define SYLVERANT_INFO_BB       0x00000020
#define SYLVERANT_INFO_XBOX     0x00000040

/* Languages that can be set for the info entries. */
#define SYLVERANT_INFO_JAPANESE 0x00000001
#define SYLVERANT_INFO_ENGLISH  0x00000002
#define SYLVERANT_INFO_GERMAN   0x00000004
#define SYLVERANT_INFO_FRENCH   0x00000008
#define SYLVERANT_INFO_SPANISH  0x00000010
#define SYLVERANT_INFO_CH_SIMP  0x00000020
#define SYLVERANT_INFO_CH_TRAD  0x00000040
#define SYLVERANT_INFO_KOREAN   0x00000080

/* Flags for the local_flags of a ship. */
#define SYLVERANT_SHIP_PMT_LIMITV2  0x00000001
#define SYLVERANT_SHIP_PMT_LIMITGC  0x00000002
#define SYLVERANT_SHIP_QUEST_RARES  0x00000004
#define SYLVERANT_SHIP_QUEST_SRARES 0x00000008
#define SYLVERANT_SHIP_PMT_LIMITBB  0x00000010

#define SYLVERANT_REG_DC            0x00000001
#define SYLVERANT_REG_DCNTE         0x00000002
#define SYLVERANT_REG_PC            0x00000004
#define SYLVERANT_REG_GC            0x00000008
#define SYLVERANT_REG_XBOX          0x00000010
#define SYLVERANT_REG_BB            0x00000020

typedef struct sylverant_dbconfig {
    char *type;
    char *host;
    char *user;
    char *pass;
    char *db;
    uint16_t port;
} sylverant_dbconfig_t;

typedef struct sylverant_info_file {
    char *desc;
    char *filename;
    uint32_t versions;
    uint32_t languages;
} sylverant_info_file_t;

typedef struct sylverant_limit_config {
    uint32_t id;
    char *name;
    char *filename;
    int enforce;
} sylverant_limit_config_t;

typedef struct sylverant_config {
    sylverant_dbconfig_t dbcfg;
    uint32_t server_ip;
    uint8_t server_ip6[16];
    uint16_t shipgate_port;
    uint8_t registration_required;
    char *shipgate_cert;
    char *shipgate_key;
    char *shipgate_ca;
    char *quests_dir;
    sylverant_limit_config_t *limits;
    int limits_count;
    int limits_enforced;
    sylverant_info_file_t *info_files;
    int info_file_count;
    char *log_dir;
    char *log_prefix;
    char *patch_dir;
    char *sg_scripts_file;
    char *lg_scripts_file;
    char *socket_dir;
} sylverant_config_t;

typedef struct sylverant_event {
    uint8_t start_month;
    uint8_t start_day;
    uint8_t end_month;
    uint8_t end_day;
    uint8_t lobby_event;
    uint8_t game_event;
} sylverant_event_t;

typedef struct sylverant_shipcfg {
    char *shipgate_host;
    uint16_t shipgate_port;

    char *name;
    char *ship_cert;
    char *ship_key;
    char *shipgate_ca;
    char *gm_file;
    sylverant_limit_config_t *limits;
    sylverant_info_file_t *info_files;
    char *quests_file;
    char *quests_dir;
    char *bans_file;
    char *scripts_file;
    char *bb_param_dir;
    char *v2_param_dir;
    char *bb_map_dir;
    char *v2_map_dir;
    char *gc_map_dir;
    char *v2_ptdata_file;
    char *gc_ptdata_file;
    char *bb_ptdata_file;
    char *v2_pmtdata_file;
    char *gc_pmtdata_file;
    char *bb_pmtdata_file;
    char *v2_rtdata_file;
    char *gc_rtdata_file;
    char *bb_rtdata_file;
    sylverant_event_t *events;
    char *smutdata_file;
    char *sg_data_dir;

    char *ship_host;
    char *ship_host6;
    uint32_t shipgate_flags;
    uint32_t local_flags;

    uint16_t base_port;
    uint16_t menu_code;

    int blocks;
    int info_file_count;
    int event_count;
    int limits_count;
    int limits_default;
    uint32_t privileges;
} sylverant_ship_t;

/* Read the configuration for the login server, shipgate, and patch server. */
extern int sylverant_read_config(const char *f, sylverant_config_t **cfg);

/* Clean up a configuration structure. */
extern void sylverant_free_config(sylverant_config_t *cfg);

/* Read the ship configuration data. You are responsible for calling the
   function to clean up the configuration. */
extern int sylverant_read_ship_config(const char *f, sylverant_ship_t **cfg);

/* Clean up a ship configuration structure. */
extern void sylverant_free_ship_config(sylverant_ship_t *cfg);

#endif /* !SYLVERANT__CONFIG_H */
