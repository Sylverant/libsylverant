/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2025 Lawrence Sebald

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

#ifndef SYLVERANT_LOG_H
#define SYLVERANT_LOG_H

#include <stdio.h>
#include <stdarg.h>

/* Values for the level parameter of the syl_logf() function.
   The default level set on the logger is SYL_LOG_INFO. */
#define SYL_LOG_TRACE   0
#define SYL_LOG_DEBUG   10
#define SYL_LOG_INFO    20
#define SYL_LOG_WARN    30
#define SYL_LOG_ERROR   40
#define SYL_LOG_CRIT    50

#define TRACE(...)  syl_logf(SYL_LOG_TRACE, __VA_ARGS__)
#define DEBUG(...)  syl_logf(SYL_LOG_DEBUG, __VA_ARGS__)
#define INFO(...)   syl_logf(SYL_LOG_INFO, __VA_ARGS__)
#define WARN(...)   syl_logf(SYL_LOG_WARN, __VA_ARGS__)
#define ERROR(...)  syl_logf(SYL_LOG_ERROR, __VA_ARGS__)
#define CRIT(...)   syl_logf(SYL_LOG_CRIT, __VA_ARGS__)

void syl_log_set_level(int level);
FILE *syl_log_set_file(FILE *fp);
void syl_logf(int level, const char *fmt, ...);
int syl_flogf(FILE *fp, int level, const char *fmt, ...);
int syl_vflogf(FILE *fp, const char *fmt, va_list args);

#endif /* !DEBUG_H */
