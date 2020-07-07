/*
 * Bitreverse tables.
 */

#ifndef __TEST_BITREV_TABLES_H
#define __TEST_BITREV_TABLES_H

#include <stdint.h>

/*
 * Tables for bit-reverse shuffle for n=128, 256, 512, 1024, 2048
 */
#define BITREV128_NPAIRS 56
#define BITREV256_NPAIRS 120
#define BITREV512_NPAIRS 240
#define BITREV1024_NPAIRS 496
#define BITREV2048_NPAIRS 992

extern const uint16_t bitrev128_pair[BITREV128_NPAIRS][2];
extern const uint16_t bitrev256_pair[BITREV256_NPAIRS][2];
extern const uint16_t bitrev512_pair[BITREV512_NPAIRS][2];
extern const uint16_t bitrev1024_pair[BITREV1024_NPAIRS][2];
extern const uint16_t bitrev2048_pair[BITREV2048_NPAIRS][2];

#endif /* __TEST_BITREV_TABLES_H */
