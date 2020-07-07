/*
 * Abstract interpretation for validating ntt_red.
 *
 * The functions here mirror those in ntt_red.h but they replace
 * arrays of integers by arrays of intervals.
 *
 * These function are intended to compute bounds on array elements
 * that occur during the ntt_red computations. They print the computed
 * intervals at every main iteration of the functions and warnings if
 * the intervals exceed [INT32_MIN, INT32_MAX].
 */

/*
 * BD: variant implementations of NTT
 *
 * All variants are specialized to Q=12289.
 * - omega denotes a primitive n-th root of unity (mod Q).
 * - psi denotes a square root of omega (mod Q).
 *
 * These variants use the reduction method introduced by
 * Longa and Naehrig, 2016.
 *
 * Here's how the reduction is defined:
 *
 * 1) write Q as 2^m * k + 1
 *    where k is an odd number
 *    define mask = (2^m -1)
 *
 * 2) given an integer x, its reduction is
 *
 *    red(x) = k * (x & mask) - (x >> m)
 *           = k * (x % 2^m) - (x / 2^m)
 *
 *   then we have red(x) == k * x modulo Q
 *
 *   To see this: we have
 *
 *     red(x) = k * r - q
 *
 *   where q and r are the quotient and remainder in
 *   the division of x by 2^m. We also have:
 *
 *      x = 2^m * q + r
 *
 *  Then
 *
 *    k * x - red(x) = 2^m * k * q + q
 *                   = q * (2^m * k + 1) = q * Q.
 *
 * Other nice properties: red(x) is cheap to compute
 * and it grows slowly.
 *
 * In our case:
 *
 *   Q is 12289
 *   m is 12
 *   k is 3
 *
 */

#ifndef NTT_RED_INTERVAL_H
#define NTT_RED_INTERVAL_H

#include <stdint.h>
#include "intervals.h"

/*****************
 * NORMALIZATION *
 ****************/

/*
 * The ntt_red functions produce 32bit coefficients
 * This function reduces all coefficients to an integer in [0 .. q-1].
 *
 * This computes the remainder modulo q (not cheap!).
 */
extern void abstract_normalize(interval_t **a, uint32_t n);

/*
 * Same thing but also multiply all coefficients by inverse(3).
 */
extern void abstract_normalize_inv3(interval_t **a, uint32_t n);

/*
 * Shift representation: convert a[i] in [0 .. q-1] to 
 * a'[i] in [-(q-1)/2, +(q-1)/2] (i.e., [-6144, +6144]).
 * a'[i] is either a[i] or a[i] - q.
 */
extern void abstract_shift_array(interval_t **a, uint32_t n);


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
extern void abstract_reduce_array(interval_t **a, uint32_t n);

/*
 * Reduce all elements of array a twice: a'[i] = red(red(a[i]))
 * The result satisfies:
 *    a'[i] == 9 * a[i] modulo Q
 *   -130 <= a'[i] <= 12413
 */
extern void abstract_reduce_array_twice(interval_t **a, uint32_t n);

/*
 * Convert to integers in the range [0, Q-1] after double reduction.
 * This checks whether a[i] is a sub-interval of [-Q, 2*Q-1]
 */
extern void abstract_correct(interval_t **a, uint32_t n);

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
extern void abstract_mul_reduce_array16(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract_mul_reduce_array(interval_t **c, uint32_t n, const interval_t **a, const interval_t **b);

/*
 * Product by a scalar + reduction
 * - a[i] = red(a[i] * c).
 * (So the result is equal to 3 * a[i] * c modulo Q).
 * To avoid overflow, we must have 
 *     -8796042698752 <= a[i] * c <= 8796093026303
 */
extern void abstract_scalar_mul_reduce_array(interval_t **a, uint32_t n, int32_t c);


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
extern void abstract_ntt_red_ct_rev2std(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_ntt_red_ct_rev2std(interval_t **a, uint32_t n, const interval_t **p);

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
extern void abstract_mulntt_red_ct_rev2std(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_mulntt_red_ct_rev2std(interval_t **a, uint32_t n, const interval_t **p);


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
 * - output: NTT stored in a, in bit-reverse order.
 */
extern void abstract_ntt_red_ct_std2rev(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_ntt_red_ct_std2rev(interval_t **a, uint32_t n, const interval_t **p);

/*
 * Version 4: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^ bitrev(j) * inverse(3)
 *
 * - output: NTT(a') in bit-reverse order
 *           where a'[i] = a[i] * psi^i
 *
 * Same conditions as above to ensure no overflow.
 */
extern void abstract_mulntt_red_ct_std2rev(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_mulntt_red_ct_std2rev(interval_t **a, uint32_t n, const interval_t **p);


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
extern void abstract_ntt_red_gs_rev2std(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_ntt_red_gs_rev2std(interval_t **a, uint32_t n, const interval_t **p);

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
extern void abstract_nttmul_red_gs_rev2std(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_nttmul_red_gs_rev2std(interval_t **a, uint32_t n, const interval_t **p);

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
extern void abstract_ntt_red_gs_std2rev(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_ntt_red_gs_std2rev(interval_t **a, uint32_t n, const interval_t **p);

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
extern void abstract_nttmul_red_gs_std2rev(interval_t **a, uint32_t n, const int16_t *p);
extern void abstract2_nttmul_red_gs_std2rev(interval_t **a, uint32_t n, const interval_t **p);



#endif /* NTT_RED_INTERVAL_H */
