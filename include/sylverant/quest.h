/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2011 Lawrence Sebald

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

#ifndef QUEST_H
#define QUEST_H

#include <inttypes.h>

#define SYLVERANT_QUEST_V1  (1 << 0)
#define SYLVERANT_QUEST_V2  (1 << 1)
#define SYLVERANT_QUEST_GC  (1 << 2)
#define SYLVERANT_QUEST_BB  (1 << 3)

#define SYLVERANT_QUEST_NORMAL      (1 << 0)
#define SYLVERANT_QUEST_BATTLE      (1 << 1)
#define SYLVERANT_QUEST_CHALLENGE   (1 << 2)

#define SYLVERANT_QUEST_BINDAT      0
#define SYLVERANT_QUEST_QST         1

typedef struct sylverant_quest {
    char name[32];
    char desc[112];

    char *long_desc;
    char *prefix;

    uint32_t versions;
    int episode;
    int event;
    int format;
} sylverant_quest_t;

typedef struct sylverant_qcat {
    char name[32];
    char desc[112];

    int quest_count;
    uint32_t type;
    sylverant_quest_t *quests;
} sylverant_quest_category_t;

typedef struct sylverant_qlist {
    sylverant_quest_category_t *cats;
    int cat_count;
} sylverant_quest_list_t;

extern int sylverant_quests_read(const char *filename,
                                 sylverant_quest_list_t *rv);

extern void sylverant_quests_destroy(sylverant_quest_list_t *list);

#endif /* !QUEST_H */
