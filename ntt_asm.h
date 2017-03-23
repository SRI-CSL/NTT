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

/****************
 *  REDUCTIONS  *
 ***************/

/*
 * Reduce all elements of array a: (i.e., a'[i] = red(a[i]))
 * - n = array size must be positive and a multiple of 16
 *
 * The resulting array a' satisfies:
 *     a'[i] == 3*a[i] modulo Q
 *  -524287 <= a'[i] <= 536573
 */
extern void reduce_array_asm(int32_t *a, uint32_t n);
extern void reduce_array_asm2(int32_t *a, uint32_t n);

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
extern void reduce_array_twice_asm2(int32_t *a, uint32_t n);

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

/*
 * Multiply a[i] by scalar c then reduce
 * - n = array size. It must be positive and a multiple of 16.
 * - the result is stored in place
 */
extern void scalar_mul_reduce_array_asm(int32_t *a, uint32_t n, int32_t c);



/******************
 *  NTT VARIANTS  *
 *****************/

/*
 * COOLEY-TUKEY: BIT-REVERSE TO STANDARD ORDER
 */

/*
 * Version 1:
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: array of powers of omega such that 
 *   p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, 4, .., n/2
 *   and j=0, ..., t-1.
 *
 * - output: a contains NTT(a) in standard order
 *
 * To get the right result (i.e., make sure there's no numerical overflow),
 * this function is intended to be called with
 *   -21499 <= a[i] <= 21499
 *    -6144 <= p[i] <= 6144
 */
extern void ntt_red_ct_rev2std_asm(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 2: combined product by powers of psi and NTT
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *
 * - output: NTT(a') in standard order
 *   where a'[i] = a[i] * psi^i
 *
 * Same conditions as above to ensure no overflow.
 */
extern void mulntt_red_ct_rev2std_asm(int32_t *a, uint32_t n, const int16_t *p);

/*
 * COOLEY-TUKEY: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 3:
 * - input: a[0 ... n-1] in standard order
 * - p: array of powers of omega such that 
 *   p[t + j] = omega^(n/2t)^bitrev(j) * inverse(3)
 *   for t=1, 2, 4, .., n/2
 *   and j=0, ..., t-1.
 *
 * - output: a contains NTT(a) in bit-reverse order
 *
 * To get the right result (i.e., make sure there's no numerical overflow),
 * this function is intended to be called with
 *   -21499 <= a[i] <= 21499
 *    -6144 <= p[i] <= 6144
 */
extern void ntt_red_ct_std2rev_asm(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 4: combined product by powers of psi and NTT
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^bitrev(j) * inverse(3)
 *
 * - output: NTT(a') in bit-reverse order
 *   where a'[i] = a[i] * psi^i
 *
 * Same conditions as above to ensure no overflow.
 */
extern void mulntt_red_ct_std2rev_asm(int32_t *a, uint32_t n, const int16_t *p);


/*
 * GENTLEMAN-SANDE: BIT-REVERSE TO STANDARD ORDER
 */

/*
 * Version 5:
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that p[t + j] = omega^(n/2t)^rev(j)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output:  NTT(a) in standard order
 *
 * Same conditions as above to ensure no overflow.
 */
extern void ntt_red_gs_rev2std_asm(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 6: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^rev(j) * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output:  a contains a' in standard order
 *            where a'[i] = NTT(a)[i] * psi^i.
 *
 * Same conditions as above to ensure no overflow.
 */
extern void nttmul_red_gs_rev2std_asm(int32_t *a, uint32_t n, const int16_t *p);


/*
 * GENTLEMAN-SANDE: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 7:
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output:  NTT(a) in bit-reverse order
 */
extern void ntt_red_gs_std2rev_asm(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 8: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard
 * - p: constant array such that 
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output:  a contains a' in reverse order
 *            where a'[i] = NTT(a)[i] * psi^i.
 */
extern void nttmul_red_gs_std2rev_asm(int32_t *a, uint32_t n, const int16_t *p);


#endif
