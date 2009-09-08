/*
    The PRS compressor/decompressor that this file refers to was originally
    written by Fuzziqer Software. The file was distributed with the message that
    it could be used in anything/for any purpose as long as credit was given. I
    have incorporated it into libsylverant for use with the Sylverant PSO server
    and related utilities.

    This file itself was not included with the distribution of the PRS code, and
    was added to facilitate the use within libsylverant.
*/

#ifndef SYLVERANT__PRS_H
#define SYLVERANT__PRS_H

#include <inttypes.h>

typedef uint32_t u32;
typedef uint8_t u8;

u32 prs_compress(void *source, void *dest, u32 size);
u32 prs_decompress(void *source, void *dest);
u32 prs_decompress_size(void *source);

#endif /* !SYLVERANT__PRS_H */
