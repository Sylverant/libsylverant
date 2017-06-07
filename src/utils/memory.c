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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "sylverant/memory.h"

/* So... Fair warning, this stuff is a bit ugly. It should work nicely enough
   though... */

/* Underlying structure that represents a reference object. This version is
   not padded out to a nice size, which is taken care of below. */
struct ref_unpadded {
    void (*dtor)(void *);
    uint32_t refcnt;
    uint32_t magic;
};

#define USZ sizeof(struct ref_unpadded)
#define PSZ 32
#define RMAGIC 0x1BADC0DE

/* Actual reference structure, to give us a nicely sized structure that we can
   use for reference counting. */
struct ref {
    struct ref_unpadded r;
    uint8_t padding[PSZ - USZ];
};

void *ref_alloc(size_t sz, void (*dtor)(void *)) {
    struct ref *r;
    uint8_t *ptr;

    assert(PSZ == sizeof(struct ref));

    /* Allocate space for the object and the reference counting overhead. */
    if(!(r = (struct ref *)malloc(sz + PSZ)))
        return NULL;

    /* Fill in the reference couting data. */
    r->r.dtor = dtor;
    r->r.refcnt = 1;
    r->r.magic = RMAGIC;

    /* Return the actual pointer to the object. */
    ptr = ((uint8_t *)r) + PSZ;
    return (void *)ptr;
}

void *ref_retain(void *r) {
    uint8_t *ptr = (uint8_t *)r;
    struct ref *rf;

    /* Make sure we're not trying to retain a NULL pointer... */
    if(!r)
        return NULL;

    /* Grab the reference counting struct. */
    rf = (struct ref *)(ptr - PSZ);

    /* Cowardly refuse to do anything if the magic isn't right. */
    if(rf->r.magic != RMAGIC)
        return NULL;

    ++rf->r.refcnt;

    return r;
}

void *ref_release(void *r) {
    uint8_t *ptr = (uint8_t *)r;
    struct ref *rf;

    /* Make sure we're not trying to release a NULL pointer... */
    if(!r)
        return NULL;

    /* Grab the reference counting struct. */
    rf = (struct ref *)(ptr - PSZ);

    /* Cowardly refuse to do anything if the magic isn't right. */
    if(rf->r.magic != RMAGIC)
        return NULL;

    /* Decrement the reference count and deallocate the object, if needed. */
    if(!--rf->r.refcnt) {
        if(rf->r.dtor)
            rf->r.dtor(r);

        free(rf);
        r = NULL;
    }

    return r;
}
