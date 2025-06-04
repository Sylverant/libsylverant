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

#define TLOG(...)   syl_logf(SYL_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define DLOG(...)   syl_logf(SYL_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define ILOG(...)   syl_logf(SYL_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define WLOG(...)   syl_logf(SYL_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define ELOG(...)   syl_logf(SYL_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define CLOG(...)   syl_logf(SYL_LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)

void syl_log_set_level(int level);
FILE *syl_log_set_file(FILE *fp);
void syl_logf(int level, const char *fn, int line, const char *fmt, ...);
int syl_flogf(FILE *fp, int level, const char *fn, int line,
              const char *fmt, ...);
int syl_vflogf(FILE *fp, int level, const char *fn, int line, const char *fmt,
               va_list args);

#endif /* !DEBUG_H */
