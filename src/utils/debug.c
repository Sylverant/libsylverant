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
#include <errno.h>
#include <stdarg.h>

#include "sylverant/debug.h"

static int min_level = DBG_LOG;

void debug_set_threshold(int level) {
    min_level = level;
}

void debug(int level, const char *fmt, ...) {
    va_list args;
    FILE *fp = stdout;

    /* Make sure we want to receive messages at this level. */
    if(level < min_level) {
        return;
    }

    /* Should we be printing to stderr? */
    if(level >= DBG_STDERR_THRESHOLD) {
        fp = stderr;
    }

    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fflush(fp);
}
