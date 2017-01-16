/*
 * Declarations for KAT
 */

#ifndef __DATA_POLY1024_H
#define __DATA_POLY1024_H

#include <stdint.h>

#define REPETITIONS 100
#define N 1025

extern int32_t a[REPETITIONS][N], b[REPETITIONS][N], c[REPETITIONS][N];

extern void build_kat(void);

#endif /* __DATA_POLY1024_H */
