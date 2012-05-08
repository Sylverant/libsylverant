/*
   Modified version Copyright (C) 2012 Lawrence Sebald

   This modified version encapsulates the MT19937 state in a structure to allow
   for multiple parallel streams. The original functions are supported by way of
   a global state structure, which is initialized on-demand.

   Also, this version uses the standard types defined in <stdint.h>, rather than
   assuming that unsigned long is 32-bits, as the original work does.

   Modified version released under the same license as the original work.
*/

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.
   Copyright (C) 2005, Mutsuo Saito
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#ifndef SYLVERANT__MTWIST_H
#define SYLVERANT__MTWIST_H

#include <stdint.h>

#define MT19937_N   624

/* State structure type */
struct mt19937_state {
    int mti;
    uint32_t mt[MT19937_N];
};

/* initializes mt[N] with a seed */
/* Modified version, returns -1 on error */
int init_genrand(uint32_t s);

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
/* Modified version, returns -1 on error */
int init_by_array(uint32_t init_key[], int key_length);

/* cleans up the global state */
void cleanup_genrand(void);

/* generates a random number on [0,0xffffffff]-interval */
uint32_t genrand_int32(void);

/* generates a random number on [0,0x7fffffff]-interval */
int32_t genrand_int31(void);

/* These real versions are due to Isaku Wada, 2002/01/09 added */
/* generates a random number on [0,1]-real-interval */
double genrand_real1(void);

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void);

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void);

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void);

/* New functions below... */
void mt19937_init(struct mt19937_state *rng, uint32_t s);
void mt19937_init_array(struct mt19937_state *rng, uint32_t a[], int len);

/* These all work the same as the ones up above, but with the specified state
   object. */
uint32_t mt19937_genrand_int32(struct mt19937_state *rng);
int32_t mt19937_genrand_int31(struct mt19937_state *rng);
double mt19937_genrand_real1(struct mt19937_state *rng);
double mt19937_genrand_real2(struct mt19937_state *rng);
double mt19937_genrand_real3(struct mt19937_state *rng);
double mt19937_genrand_res53(struct mt19937_state *rng);

#endif /* !SYLVERANT__MTWIST_H */
