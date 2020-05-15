/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2011, 2019, 2020 Lawrence Sebald

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
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "sylverant/debug.h"

static int min_level = DBG_LOG;
static FILE *dfp = NULL;

void debug_set_threshold(int level) {
    min_level = level;
}

FILE *debug_set_file(FILE *fp) {
    FILE *ofp = dfp;

    if(fp)
        dfp = fp;

    return ofp;
}

void debug(int level, const char *fmt, ...) {
    va_list args;

    /* If the default file we write to hasn't been initialized, set it to
       stdout. */
    if(!dfp)
        dfp = stdout;

    /* Make sure we want to receive messages at this level. */
    if(level < min_level)
        return;

    va_start(args, fmt);
    vfdebug(dfp, fmt, args);
    va_end(args);
}

int fdebug(FILE *fp, int level, const char *fmt, ...) {
    va_list args;
    int rv;

    if(!fp || !fmt)
        return -1;

    if(level < min_level)
        return 0;

    va_start(args, fmt);
    rv = vfdebug(fp, fmt, args);
    va_end(args);

    return rv;
}

int vfdebug(FILE *fp, const char *fmt, va_list args) {
    struct timeval rawtime;
    struct tm cooked;

    if(!fp || !fmt)
        return -1;

    /* Get the timestamp */
    gettimeofday(&rawtime, NULL);

    /* Get UTC */
    gmtime_r(&rawtime.tv_sec, &cooked);

    /* Print the timestamp */
    fprintf(fp, "[%u:%02u:%02u: %02u:%02u:%02u.%03u]: ", cooked.tm_year + 1900,
            cooked.tm_mon + 1, cooked.tm_mday, cooked.tm_hour, cooked.tm_min,
            cooked.tm_sec, (unsigned int)(rawtime.tv_usec / 1000));

    vfprintf(fp, fmt, args);
    fflush(fp);
    return 0;
}
