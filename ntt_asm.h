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

/*
 * Reduce all elements of array a: (i.e., a'[i] = red(a[i]))
 * - n = array size must be positive and a multiple of 16
 *
 * The resulting array a' satisfies:
 *     a'[i] == 3*a[i] modulo Q
 *  -524287 <= a'[i] <= 536573
 */
extern void reduce_array_asm(int32_t *a, uint32_t n);

/*
 * Reduce all elements of array a twice: a[i] = red(red(a[i]))
 * - n = array size: it must be positive and a multiple of 16
 *
 * The result is stored in place.
 *
 * The result satisfies:
 *    a'[i] == 9 * a[i] modulo Q
 *   -130 <= a'[i] <= 12413
 */
extern void reduce_array_twice_asm(int32_t *a, uint32_t n);

/*
 * Convert to integers in the range [0, Q-1] after double reduction.
 * - n must be positive and a multiple of 16
 * - the input must satisfy -Q <= a[i] <= 2*Q-1
 */
extern void correct_asm(int32_t *a, uint32_t n);

/*
 * Multiply a[i] by p[i] then reduce the result.
 * - a is modified in place
 * - n = size of both arrays, must be positive and a multiple of 16
 */
extern void mul_reduce_array16_asm(int32_t *a, uint32_t n, const int16_t *p);
extern void mul_reduce_array16_asm2(int32_t *a, uint32_t n, const int16_t *p);


/*
 * Multiply b[i] by c[i] then reduce and store the result in a[i]
 * - n = size of the arrays. It must be positive and a multiple of 16.
 */
extern void mul_reduce_array_asm(int32_t *a, uint32_t n, const int32_t *b, const int32_t *c);

#endif
