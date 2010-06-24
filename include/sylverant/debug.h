/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2009 Lawrence Sebald

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

#ifndef DEBUG_H
#define DEBUG_H

/* Values for the level parameter of the debug function. */
#define DBG_LOG         1
#define DBG_NORMAL      2
#define DBG_WARN        10
#define DBG_ERROR       20

/* Threshold to print to stderr */
#define DBG_STDERR_THRESHOLD DBG_WARN

void debug_set_threshold(int level);
void debug(int level, const char *fmt, ...);

#endif /* !DEBUG_H */
