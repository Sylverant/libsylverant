/*
   Modified version Copyright (C) 2012 Lawrence Sebald

   This modified version encapsulates the MT19937 state in a structure to allow
   for multiple parallel streams. The original functions are supported by way of
   a global state structure.

   Modified version released under the same license as the original work.
*/

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          
   Copyright (C) 2005, Mutsuo Saito,
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

#include <stdio.h>
#include <stdlib.h>
#include "sylverant/mtwist.h"

/* Period parameters */  
#define N MT19937_N
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static struct mt19937_state *gstate = NULL;

void mt19937_init(struct mt19937_state *rng, uint32_t s) {
    rng->mt[0]= s & 0xffffffffUL;
    for (rng->mti=1; rng->mti<N; rng->mti++) {
        rng->mt[rng->mti] = 
	    (1812433253UL * (rng->mt[rng->mti-1] ^
                         (rng->mt[rng->mti-1] >> 30)) + rng->mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        rng->mt[rng->mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

void mt19937_init_array(struct mt19937_state *rng, uint32_t a[], int len) {
    int i, j, k;

    mt19937_init(rng, 19650218UL);

    i=1; j=0;
    k = (N>len ? N : len);
    for (; k; k--) {
        rng->mt[i] = (rng->mt[i] ^ ((rng->mt[i-1]
            ^ (rng->mt[i-1] >> 30)) * 1664525UL))
            + a[j] + j; /* non linear */
        rng->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { rng->mt[0] = rng->mt[N-1]; i=1; }
        if (j>=len) j=0;
    }
    for (k=N-1; k; k--) {
        rng->mt[i] = (rng->mt[i] ^ ((rng->mt[i-1]
            ^ (rng->mt[i-1] >> 30)) * 1566083941UL))
            - i; /* non linear */
        rng->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { rng->mt[0] = rng->mt[N-1]; i=1; }
    }

    rng->mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

uint32_t mt19937_genrand_int32(struct mt19937_state *rng) {
    uint32_t y;
    uint32_t mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (rng->mti >= N) { /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (rng->mt[kk]&UPPER_MASK)|(rng->mt[kk+1]&LOWER_MASK);
            rng->mt[kk] = rng->mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (rng->mt[kk]&UPPER_MASK)|(rng->mt[kk+1]&LOWER_MASK);
            rng->mt[kk] = rng->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (rng->mt[N-1]&UPPER_MASK)|(rng->mt[0]&LOWER_MASK);
        rng->mt[N-1] = rng->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        rng->mti = 0;
    }

    y = rng->mt[rng->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

int32_t mt19937_genrand_int31(struct mt19937_state *rng) {
    return (int32_t)(mt19937_genrand_int32(rng) >> 1);
}

double mt19937_genrand_real1(struct mt19937_state *rng) {
    return mt19937_genrand_int32(rng) * (1.0 / 4294967295.0); 
}

double mt19937_genrand_real2(struct mt19937_state *rng) {
    return mt19937_genrand_int32(rng) * (1.0 / 4294967296.0); 
}

double mt19937_genrand_real3(struct mt19937_state *rng) {
    return (((double)mt19937_genrand_int32(rng)) + 0.5) * (1.0 / 4294967296.0); 
}

/* generates a random number on [0,1) with 53-bit resolution*/
double mt19937_genrand_res53(struct mt19937_state *rng) { 
    uint32_t a = mt19937_genrand_int32(rng) >> 5;
    uint32_t b = mt19937_genrand_int32(rng) >> 6; 
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0); 
} 

/* initializes mt[N] with a seed */
int init_genrand(uint32_t s)
{
    if (!gstate) {
        gstate = (struct mt19937_state *)malloc(sizeof(struct mt19937_state));
        if (!gstate)
            return -1;
    }

    mt19937_init(gstate, s);

    return 0;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
int init_by_array(uint32_t init_key[], int key_length)
{
    if (!gstate) {
        gstate = (struct mt19937_state *)malloc(sizeof(struct mt19937_state));
        if (!gstate)
            return -1;
    }

    mt19937_init_array(gstate, init_key, key_length);

    return 0;
}

/* cleans up the initialized state */
void cleanup_genrand(void) {
    free(gstate);
    gstate = NULL;
}

/* generates a random number on [0,0xffffffff]-interval */
uint32_t genrand_int32(void)
{
    if (!gstate)   /* if init_genrand() has not been called, */
        init_genrand(5489UL); /* a default initial seed is used */

    if (!gstate)   /* if the state doesn't initialize, what to do? */
        return (uint32_t)-1;
}

/* generates a random number on [0,0x7fffffff]-interval */
int32_t genrand_int31(void)
{
    return (int32_t)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) 
{ 
    uint32_t a=genrand_int32()>>5, b=genrand_int32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */
