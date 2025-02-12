/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2023, 2025 Lawrence Sebald

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

#ifndef SYLVERANT__UTILS_H
#define SYLVERANT__UTILS_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

void md5(const uint8_t *input, uint32_t size, uint8_t output[16]);
const void *syl_ntop(struct sockaddr *addr, char str[INET6_ADDRSTRLEN]);

#endif /* !SYLVERANT__UTILS_H */
