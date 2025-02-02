/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2011, 2025 Lawrence Sebald

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

#include <stddef.h>
#include <arpa/inet.h>
#include "sylverant/utils.h"

const void *syl_ntop(struct sockaddr *addr, char str[INET6_ADDRSTRLEN]) {
    int family = addr->sa_family;

    switch(family) {
        case AF_INET:
        {
            struct sockaddr_in *a = (struct sockaddr_in *)addr;
            return inet_ntop(family, &a->sin_addr, str, INET6_ADDRSTRLEN);
        }

        case AF_INET6:
        {
            struct sockaddr_in6 *a = (struct sockaddr_in6 *)addr;
            return inet_ntop(family, &a->sin6_addr, str, INET6_ADDRSTRLEN);
        }
    }

    return NULL;
}
