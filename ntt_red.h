/*
 * BD: variant implementations of NTT
 *
 * All variants are specialized to Q=12289.
 * - omega denotes a primitive n-th root of unity (mod Q).
 * - psi denotes a square root of omega (mod Q).
 *
 * These variants use the reduction method introduced by
 * Longa and Naehrig, 2016.
 */

#ifndef __NTT_RED_H
#define __NTT_RED_H

#include <stdint.h>


/*****************
 * NORMALIZATION *
 ****************/

/*
 * The ntt_red functions produce 32bit coefficients
 * This function reduces all coefficients to an integer in [0 .. q-1].
 *
 * This computes the remainder modulo q (not cheap!).
 */
extern void normalize(int32_t *a, uint32_t n);

/*
 * Same thing but also multiply all coefficients by inverse(3).
 */
extern void normalize_inv3(int32_t *a, uint32_t n);


/*
 * Shift representation: convert a[i] in [0 .. q-1] to 
 * a'[i] in [-(q-1)/2, +(q-1)/2] (i.e., [-6144, +6144]).
 * a'[i] is either a[i] or a[i] - q.
 */
extern void shift_array(int32_t *a, uint32_t n);


/**************
 * REDUCTIONS *
 *************/

/*
 * Reduce all elements of array a: (i.e., a'[i] = red(a[i]))
 * The resulting array a' satisfies:
 *     a'[i] == 3*a[i] modulo Q
 *  -524287 <= a'[i] <= 536573
 *
 * In particular, we can do this:
 *
 *   reduce_array(a, n)
 *   reduce_array(b, n)
 *   mul_reduce_array(c, n, a, b)
 *
 * and the mul_reduce won't overflow.
 */
extern void reduce_array(int32_t *a, uint32_t n);

/*
 * Reduce all elements of array a twice: a'[i] = red(red(a[i]))
 * The result satisfies:
 *    a'[i] == 9 * a[i] modulo Q
 *   -130 <= a'[i] <= 12413
 */
extern void reduce_array_twice(int32_t *a, uint32_t n);

/*
 * Convert to integers in the range [0, Q-1] after double reduction.
 */
extern void correct(int32_t *a, uint32_t n);

/*
 * Multiply a[i] by p[i] then reduce
 * The result satisfies:
 *    a'[i] == 3 * a[i] * p[i] modulo Q
 *
 * Bounds on a'[i]:
 * 1) if 0 <= a[i] <= 12288 and 0 <= p[i] <= 12288 then
 *      -36864 <= a'[i] <= 12285
 *
 * 2) if -6144 <= a[i] <= 6144 and -6144 <= p[i] <= 6144 then
 *      -9216 <= a'[i] <= 21499
 *
 * In particular, if condition (2) holds, then the result is a safe input to 
 * the ntt functions.
 * 
 * mul_reduce_array16 does the operation in-place, with p an array of 16bit constants.
 * It computes a[i] * p[i] using 64bit arithmetic and reduce the result to 32bits.
 *
 * mul_array builds the reduced product in array c from two 32bit arrays a and b.
 * - a[i] * b[i] is computed using 64bit arithmetic but the reduced value is
 *   converted to 32bits.
 * To avoid overflow, we must have 
 *     -8796042698752 <= a[i] * b[i] <= 8796093026303
 */
extern void mul_reduce_array16(int32_t *a, uint32_t n, const int16_t *p);
extern void mul_reduce_array(int32_t *c, uint32_t n, const int32_t *a, const int32_t *b);

/*
 * Product by a scalar + reduction
 * - a[i] = red(a[i] * c).
 * (So the result is equal to 3 * a[i] * c modulo Q).
 * To avoid overflow, we must have 
 *     -8796042698752 <= a[i] * c <= 8796093026303
 */
extern void scalar_mul_reduce_array(int32_t *a, uint32_t n, int32_t c);


/****************
 * NTT VARIANTS *
 ***************/

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
extern void ntt_red_ct_rev2std(int32_t *a, uint32_t n, const int16_t *p);

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
extern void mulntt_red_ct_rev2std(int32_t *a, uint32_t n, const int16_t *p);


/*
 * COOLEY-TUKEY: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 3:
 * - input a[0 ... n-1] in standard order
 * - p: constant array such that
 *   p[t + j] = omega^(n/2t)^ bitrev(j) * inverse(3)
 *   for t=1, 2, 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT stored in a, in standard order.
 */
extern void ntt_red_ct_std2rev(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 4: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^ bitrev(j) * inverse(3)
 *
 * - output: NTT(a') in standard order
 *           where a'[i] = a[i] * psi^i
 *
 * Same conditions as above to ensure no overflow.
 */
extern void mulntt_red_ct_std2rev(int32_t *a, uint32_t n, const int16_t *p);


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
extern void ntt_red_gs_rev2std(int32_t *a, uint32_t n, const int16_t *p);

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
extern void nttmul_red_gs_rev2std(int32_t *a, uint32_t n, const int16_t *p);


/*
 * GENTLEMAN-SANDE: STANDARD TO BIT-REVERSE ORDER
 */

/*
 * Version 7:
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in bit-reverse order
 *
 * Same conditions as above to ensure no overflow.
 */
extern void ntt_red_gs_std2rev(int32_t *a, uint32_t n, const int16_t *p);

/*
 * Version 8: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: a contains a' in reverse order
 *           where a'[i] = NTT(a)[i] * psi^i.
 *
 * Same conditions as above to ensure no overflow.
 */
extern void nttmul_red_gs_std2rev(int32_t *a, uint32_t n, const int16_t *p);

#endif /* __NTT_RD_H */
