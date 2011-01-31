/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2010 Lawrence Sebald

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

/* SYLVERANT_DIRECTORY is now dependent on the DATAROOTDIR variable that should
   be set by the build system. If it is not set, we'll fall back on the old
   default of /usr/local/share . Since this will only be set when building the
   library, and not the rest of the tools, we have this file now with constant
   strings holding the directories. */

#ifndef DATAROOTDIR
#define DATAROOTDIR "/usr/local/share"
#endif

#define SYLVERANT_DIRECTORY DATAROOTDIR "/sylverant"

const char sylverant_directory[] = SYLVERANT_DIRECTORY;
const char sylverant_cfg[] = SYLVERANT_DIRECTORY "/config/sylverant_config.xml";
const char sylverant_ship_cfg[] = SYLVERANT_DIRECTORY "/config/ship_config.xml";
