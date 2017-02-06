/*
 * BD: variant implementations of NTT
 *
 * All variants are specialized to Q=12289.
 * - omega denotes a primitive n-th root of unity (mod Q).
 * - psi denotes a square root of omega (mod Q).
 *
 * These variants use the reduction method introduced by
 * Longa and Naehrig, 2016. The implementation uses intel's
 * AVX2 vector instructions.
 */

#ifndef __NTT_ASM_H
#define __NTT_ASM_H

#include <stdint.h>
#include <stdbool.h>


/*
 * Check whether AVX2 is supported
 */
extern bool avx2_supported(void);

#endif
