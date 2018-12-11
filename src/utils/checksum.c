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

#include "sylverant/checksum.h"

/* Calculate a CRC32 checksum over a given block of data. Somewhat inspired by
   the CRC32 function in Figure 14-6 of http://www.hackersdelight.org/crc.pdf */
uint32_t sylverant_crc32(const uint8_t *data, int size) {
    int i, j;
    uint32_t rv = 0xFFFFFFFF;

    for(i = 0; i < size; ++i) {
        rv ^= data[i];
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^ (rv >> 1);
    }

    return ~rv;
}
