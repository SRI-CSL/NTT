/*
 * BD: variant implementations of NTT
 *
 * Naive implementation: to prevent the compiler from
 * optimizing mod Q, we pass q as an extra argument.
 *
 * - we always call with Q = 12289
 * - omega denotes a primitive n-th root of unity (mod Q).
 * - psi denotes a square root of omega (mod Q).
 */

#ifndef __NTT_H
#define __NTT_H

#include <stdint.h>


/*************
 * UTILITIES *
 ************/

/*
 * Shuffle a: swap a[i] and a[j] where j = bit-reverse i
 * - generic version for arbitrary n
 */
extern void bitrev_shuffle(int32_t *a, uint32_t n);

/*
 * Bit-Reverse using a pre-computed table of pairs [i, j] with i<j and j = bit-reverse(i)
 * - p = the table
 * - n = number of pairs in p
 */
extern void shuffle_with_table(int32_t *a, const uint16_t p[][2], uint32_t n);

/*
 * In-place elementwise product: a[i] = a[i] * p[i] mod Q
 * - a[i] and p[i] must be between 0 and Q-1
 * - p is assumed to be a array of 16bit constants (e.g., p[i] = psi^i mod Q)
 */
extern void mul_array16_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

/*
 * Elementwise product: c[i] = a[i] * b[i]
 * - all arrays are 32bit integers of size n
 * - a[i] and b[i] must be between 0 and Q-1
 */
extern void mul_array_naive(int32_t *c, uint32_t n, const int32_t *a, const int32_t *b, int32_t q);

/*
 * Product by a scalar c: a[i] = c * a[i]
 * - a[i] must be between 0 and Q-1
 * - c must be between 0 and Q-1
 * - the product is done in place.
 */
extern void scalar_mul_array_naive(int32_t *a, uint32_t n, int32_t c, int32_t q);


/****************
 * NTT VARIANTS *
 ***************/

/*
 * COOLEY-TUKEY: BIT-REVERSE TO STANDARD ORDER
 */

/*
 * Version 2: different precomputed powers
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: array of powers of omega
 *   such that p[t + j] = omega^(n/2t)^j
 *   for t=1, 2, 4, .., n/2
 *   and j=0, ..., t-1.
 *
 * This array looks like this:
 *   p[0] --> not used
 *   p[1] --> 1
 *   p[2] --> 1, omega^(n/4)
 *   p[4] --> 1, omega^(n/8), omega^(2*n/8), omega^(3*n/8)
 *   p[8] --> 1, omega^(n/16), ....
 *
 * - output: a contains the NTT(a) in standard order
 */
extern void ntt_ct_rev2std_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

/*
 * Version 3: combined product by powers of psi and NTT
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^j
 *
 * - output: NTT(a') in standard order, where a'[i] = a[i] * psi^i
 */
extern void mulntt_ct_rev2std_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);


/*
 * COOLEY-TUKEY: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 4
 * - input a[0 ... n-1] in standard order
 * - p: constant array such that
 *   p[t + j] = omega^(n/2t)^ bitrev(j)
 *   for t=1, 2, 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a) in bit-reverse order
 */
extern void ntt_ct_std2rev_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

/*
 * Version 5: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^ bitrev(j)
 *
 * - output: NTT(a') in bit-reverse order, where a'[i] = a[i] * psi^i
 */
extern void mulntt_ct_std2rev_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);


/*
 * GENTLEMAN-SANDE: BIT-REVERSE TO STANDARD ORDER
 */

/*
 * Version 6
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that p[t + j] = omega^(n/2t)^rev(j)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in standard order
 */
extern void ntt_gs_rev2std_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

/*
 * Version 7: combined NTT and product by powers of psi, using GS
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that p[t + j] = psi^(n/2t) * omega^(n/2t)^rev(j)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in standard order with each element multiplied by
 *   a power of psi (i.e., a'[i] = NTT(a)[i] * psi^i).
 */
extern void nttmul_gs_rev2std_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);


/*
 * GENTLEMAN-SANDE: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 8
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = omega^(n/2t)^j
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in reverse order
 */
extern void ntt_gs_std2rev_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

/*
 * Version 8: combined NTT and product by powers of psi, using GS
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = psi^(n/2t) * omega^(n/2t)^j
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in reverse order, with each element multiplied by
 *   a power of psi (i.e., a'[i] = NTT(a)[rev(i)] * psi^rev(i)].
 */
extern void nttmul_gs_std2rev_naive(int32_t *a, uint32_t n, const uint16_t *p, int32_t q);

#endif /* __NTT_H */
