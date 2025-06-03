/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009, 2011, 2019, 2020, 2025 Lawrence Sebald

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

#include "sylverant/log.h"

static int min_level = SYL_LOG_INFO;
static FILE *dfp = NULL;

static const char *levels[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRIT"
};

static const char *levelname(int level) {
    if((level % 10) == 0 && level >= 0 && level <= 50) {
        return levels[level / 10];
    }

    return NULL;
}

void syl_log_set_level(int level) {
    min_level = level;
}

FILE *syl_log_set_file(FILE *fp) {
    FILE *ofp = dfp;

    if(fp)
        dfp = fp;

    return ofp;
}

void syl_logf(int level, const char *fn, int line, const char *fmt, ...) {
    va_list args;

    /* If the default file we write to hasn't been initialized, set it to
       stdout. */
    if(!dfp)
        dfp = stdout;

    va_start(args, fmt);
    syl_vflogf(dfp, level, fn, line, fmt, args);
    va_end(args);
}

int syl_flogf(FILE *fp, int level, const char *fn, int line,
              const char *fmt, ...) {
    va_list args;
    int rv;

    if(!fp || !fmt)
        return -1;

    va_start(args, fmt);
    rv = syl_vflogf(fp, level, fn, line, fmt, args);
    va_end(args);

    return rv;
}

int syl_vflogf(FILE *fp, int level, const char *fn, int line, const char *fmt,
               va_list args) {
    struct timeval rawtime;
    struct tm cooked;
    char timestamp[200];
    const char *lname;

    if(!fp || !fmt)
        return -1;

    if(level < min_level)
        return 0;

    lname = levelname(level);

    /* Get the timestamp */
    gettimeofday(&rawtime, NULL);

    /* Get UTC */
    gmtime_r(&rawtime.tv_sec, &cooked);

    /* Print the timestamp and level of the log in common log format style... */
    strftime(timestamp, 200, "%d/%b/%Y:%H:%M:%S %z", &cooked);

    if(lname)
        fprintf(fp, "[%s:%d] [%s] [%s]: ", fn, line, timestamp, lname);
    else
        fprintf(fp, "[%s:%d] [%s] [%d]: ", fn, line, timestamp, level);

    vfprintf(fp, fmt, args);
    fflush(fp);
    return 0;
}
