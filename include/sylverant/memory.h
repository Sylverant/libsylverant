/*
    This file is part of Sylverant PSO Server.

    Copyright (C) 2014 Lawrence Sebald

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

#ifndef SYLVERANT__MEMORY_H
#define SYLVERANT__MEMORY_H

#include <stddef.h>

extern void *ref_alloc(size_t sz, void (*dtor)(void *));
extern void *ref_retain(void *r);
extern void *ref_release(void *r);

#define retain(x) ref_retain(x)
#define release(x) ref_release(x)

#endif /* !SYLVERANT__MEMORY_H */
