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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <errno.h>

#include "sylverant/quest.h"

#define BUF_SIZE 8192
#define DIE() XML_StopParser(parser, 0); return

static XML_Parser parser = NULL;
static sylverant_quest_category_t *cat = NULL;
static sylverant_quest_t *quest = NULL;
static int text_type = -1;

static void q_start_hnd(void *d, const XML_Char *name, const XML_Char **attrs) {
    int i;
    sylverant_quest_list_t *l = (sylverant_quest_list_t *)d;
    void *tmp;

    if(!strcmp(name, "category")) {
        /* Make sure we should expect a <category> here */
        if(cat != NULL) {
            DIE();
        }

        /* Allocate space for the new category */
        tmp = realloc(l->cats, (l->cat_count + 1) *
                      sizeof(sylverant_quest_category_t));

        /* Make sure we got the space */
        if(!tmp) {
            DIE();
        }

        l->cats = (sylverant_quest_category_t *)tmp;

        /* Grab the pointer to this category */
        cat = l->cats + l->cat_count++;

        /* Clear the category */
        memset(cat, 0, sizeof(sylverant_quest_category_t));
        cat->type = SYLVERANT_QUEST_NORMAL;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "name")) {
                strncpy(cat->name, attrs[i + 1], 31);
                cat->name[31] = '\0';
            }
            else if(!strcmp(attrs[i], "type")) {
                if(!strcmp(attrs[i + 1], "normal")) {
                    cat->type = SYLVERANT_QUEST_NORMAL;
                }
                else if(!strcmp(attrs[i + 1], "battle")) {
                    cat->type = SYLVERANT_QUEST_BATTLE;
                }
                else if(!strcmp(attrs[i + 1], "challenge")) {
                    cat->type = SYLVERANT_QUEST_CHALLENGE;
               }
            }
            else {
                DIE();
            }
        }
    }
    else if(!strcmp(name, "description")) {
        /* Make sure we're expecting a <description> here */
        if(cat == NULL || quest != NULL) {
            DIE();
        }

        /* Set up the text type */
        text_type = 0;
    }
    else if(!strcmp(name, "quest")) {
        /* Make sure we're expecting a <quest> here */
        if(cat == NULL || quest != NULL) {
            DIE();
        }

        /* Allocate space for the new quest */
        tmp = realloc(cat->quests, (cat->quest_count + 1) *
                      sizeof(sylverant_quest_t));

        /* Make sure we got the space */
        if(!tmp) {
            DIE();
        }

        cat->quests = (sylverant_quest_t *)tmp;

        /* Grab the pointer to this quest */
        quest = cat->quests + cat->quest_count++;

        /* Clear the quest */
        memset(quest, 0, sizeof(sylverant_quest_t));

        /* Default to episode 1 and always available */
        quest->episode = 1;
        quest->event = -1;

        for(i = 0; attrs[i]; i += 2) {
            if(!strcmp(attrs[i], "name")) {
                strncpy(quest->name, attrs[i + 1], 31);
                quest->name[31] = '\0';
            }
            else if(!strcmp(attrs[i], "v1")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    quest->versions |= SYLVERANT_QUEST_V1;
                }
            }
            else if(!strcmp(attrs[i], "v2")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    quest->versions |= SYLVERANT_QUEST_V2;
                }
            }
            else if(!strcmp(attrs[i], "gc")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    quest->versions |= SYLVERANT_QUEST_GC;
                }
            }
            else if(!strcmp(attrs[i], "bb")) {
                if(!strcmp(attrs[i + 1], "true")) {
                    quest->versions |= SYLVERANT_QUEST_BB;
                }
            }
            else if(!strcmp(attrs[i], "prefix")) {
                /* Only pay attention to the first of these we see */
                if(quest->prefix == NULL) {
                    quest->prefix = (char *)malloc(strlen(attrs[i + 1]) + 1);

                    if(!quest->prefix) {
                        DIE();
                    }

                    strcpy(quest->prefix, attrs[i + 1]);
                }
            }
            else if(!strcmp(attrs[i], "episode")) {
                quest->episode = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "event")) {
                quest->event = atoi(attrs[i + 1]);
            }
            else if(!strcmp(attrs[i], "format")) {
                if(!strcmp(attrs[i + 1], "qst")) {
                    quest->format = SYLVERANT_QUEST_QST;
                }
                else if(!strcmp(attrs[i + 1], "bin/dat")) {
                    quest->format = SYLVERANT_QUEST_BINDAT;
                }
                else {
                    DIE();
                }
            }
            else if(!strcmp(attrs[i], "id")) {
                errno = 0;
                quest->qid = (uint32_t)strtoul(attrs[i + 1], NULL, 0);

                if(errno) {
                    DIE();
                }
            }
            else {
                DIE();
            }
        }
    }
    else if(!strcmp(name, "short")) {
        /* Make sure we're expecting a <short> here */
        if(cat == NULL || quest == NULL) {
            DIE();
        }

        /* Set up the text type */
        text_type = 1;
    }
    else if(!strcmp(name, "long")) {
        /* Make sure we're expecting a <long> here */
        if(cat == NULL || quest == NULL) {
            DIE();
        }

        /* Set up the text type */
        text_type = 2;
    }
}

static void q_end_hnd(void *d, const XML_Char *name) {
    if(!strcmp(name, "quest")) {
        quest = NULL;
    }
    else if(!strcmp(name, "category")) {
        cat = NULL;
    }
    else if(!strcmp(name, "description") || !strcmp(name, "short") ||
            !strcmp(name, "long")) {
        text_type = -1;
    }
}

static void q_text_hnd(void *d, const XML_Char *s, int len) {
    int slen, cplen;
    void *tmp;

    /* Ignore text if its not in a <description> <short> or <long> */
    if(text_type == -1) {
        return;
    }

    /* Check what kind of tag we have */
    if(text_type == 0) {
        /* We have a <description> tag */
        slen = strlen(cat->desc);
        cplen = 111 - slen;

        if(len < cplen) {
            cplen = len;
        }

        if(cplen) {
            memcpy(cat->desc + slen, s, cplen);
        }
    }
    else if(text_type == 1) {
        /* We have a <short> tag */
        slen = strlen(quest->desc);
        cplen = 111 - slen;

        if(len < cplen) {
            cplen = len;
        }

        if(cplen) {
            memcpy(quest->desc + slen, s, cplen);
        }
    }
    else if(text_type == 2) {
        /* We have a <long> tag */
        if(quest->long_desc) {
            slen = strlen(quest->long_desc);
            tmp = realloc(quest->long_desc, slen + len + 1);
        }
        else {
            slen = 0;
            tmp = malloc(len + 1);
        }

        /* Make sure we got the memory */
        if(!tmp) {
            DIE();
        }

        /* Save the pointer where it belongs */
        quest->long_desc = (char *)tmp;

        /* Copy the new string in */
        memcpy(quest->long_desc + slen, s, len);

        /* NUL terminate it. */
        quest->long_desc[slen + len] = 0;
    }
}

int sylverant_quests_read(const char *filename, sylverant_quest_list_t *rv) {
    FILE *fp;
    XML_Parser p;
    int bytes;
    void *buf;

    /* Clear out the config. */
    memset(rv, 0, sizeof(sylverant_quest_list_t));

    /* Open the configuration file for reading. */
    fp = fopen(filename, "r");

    if(!fp) {
        return -1;
    }

    /* Create the XML parser object. */
    p = XML_ParserCreate(NULL);

    if(!p)  {
        fclose(fp);
        return -2;
    }

    /* Set the callbacks that expat needs */
    XML_SetElementHandler(p, &q_start_hnd, &q_end_hnd);
    XML_SetCharacterDataHandler(p, &q_text_hnd);

    /* Set up the user data so we can store the configuration. */
    XML_SetUserData(p, rv);

    parser = p;

    for(;;) {
        /* Grab the buffer to read into. */
        buf = XML_GetBuffer(p, BUF_SIZE);

        if(!buf)    {
            printf("%s\n", XML_ErrorString(XML_GetErrorCode(p)));
            printf("\tAt: %d:%d\n", (int)XML_GetCurrentLineNumber(p),
                   (int)XML_GetCurrentColumnNumber(p));
            XML_ParserFree(p);
            fclose(fp);
            parser = NULL;
            cat = NULL;
            quest = NULL;
            text_type = -1;
            return -2;
        }

        /* Read in from the file. */
        bytes = fread(buf, 1, BUF_SIZE, fp);

        if(bytes < 0)   {
            XML_ParserFree(p);
            fclose(fp);
            parser = NULL;
            cat = NULL;
            quest = NULL;
            text_type = -1;
            return -2;
        }

        /* Parse the bit we read in. */
        if(!XML_ParseBuffer(p, bytes, !bytes))  {
            printf("%s\n", XML_ErrorString(XML_GetErrorCode(p)));
            printf("\tAt: %d:%d\n", (int)XML_GetCurrentLineNumber(p),
                   (int)XML_GetCurrentColumnNumber(p));
            XML_ParserFree(p);
            fclose(fp);
            parser = NULL;
            cat = NULL;
            quest = NULL;
            text_type = -1;
            return -3;
        }

        if(!bytes)  {
            break;
        }
    }

    XML_ParserFree(p);
    fclose(fp);

    parser = NULL;
    cat = NULL;
    quest = NULL;
    text_type = -1;

    return 0;
}

void sylverant_quests_destroy(sylverant_quest_list_t *list) {
    int i, j;
    sylverant_quest_category_t *cat;
    sylverant_quest_t *quest;

    for(i = 0; i < list->cat_count; ++i) {
        cat = &list->cats[i];

        for(j = 0; j < cat->quest_count; ++j) {
            quest = &cat->quests[j];

            /* Free each malloced item in the quest. */
            free(quest->long_desc);
            free(quest->prefix);
        }

        /* Free the list of quests. */
        free(cat->quests);
    }

    /* Free the list of categories, and we're done. */
    free(list->cats);
    list->cats = NULL;
    list->cat_count = 0;
}
