/*
 * Abstract interpretation for variant implementations of NTT and
 * Inverse NTT. All abstract_ntt functions here are similar to
 * the original functions in ../ntt_red.c but they work in an
 * abstract interval domain.
 *
 * All inputs are arrays of intervals (i.e., assumed bounds on the
 * input values). The 'abstract' functions compute bounds on every
 * array element when the functions return. They print the final
 * bounds and intermediate results after every main iteration in each
 * function. They also report integer overflow when the intervals are
 * not included in [INT32_MIN, INT32_MAX].  This means that there's a
 * risk of integer overflow, since the ntt_red functions use 32bit
 * arithmetic.
 *
 * NOTE: There could be false alarms (as we overapproximate the
 * intervals).
 * 
 *
 * Example usage:
 * - we want to show that there's no risk of overflow in 
 *   ntt_red_ct_rev2std(a, 1024, omega_powers)
 *   provide we have 0 <= a[i] <= 12288 for every i
 *
 * - to check this, we call
 *   abstract_ntt_red_ct_rev2std(a, 1024, omega_powers)
 *   on an array  a of intervals such that
 *    a[i] = [0, 12288].
 *
 * - this is done in test_abstract1024.c:
 *
 *     interval_t *a[1024];
 *     uint32_t i;
 *     for (i=0; i<1024; i++) {
 *        a[i] = interval(0, 12288);
 *     }
 *     abstract_ntt_red_ct_rev2std(a, 1024, ntt_red1024_omega_powers);
 *
 *  It prints bounds on a[0] .... a[1023] after every iteration.
 *  And the final bounds show that there's no overflow possible.
 *
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ntt_red_interval.h"

/*
 * NORMALIZATION
 */

/*
 * The ntt_red functions produce 32bit coefficients
 * This function reduces all coefficients to integers in [0 .. q-1]
 */
void abstract_normalize(interval_t **a, uint32_t n) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = normal(x);
    delete_interval(x);
  }
}

/*
 * Same thing but also multiply all coefficients by inverse(3).
 */
void abstract_normalize_inv3(interval_t **a, uint32_t n) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = normal_inv3(x);
    delete_interval(x);
  }
}


/*
 * Shift representation: from [0 .. Q-1] to [-(Q-1)/2 .. +(Q-1)/2]
 */
void abstract_shift_array(interval_t **a, uint32_t n) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = shift(x);
    delete_interval(x);
  }  
}


/*
 * REDUCTIONS
 */

/*
 * Reduce all elements of array a: (i.e., a'[i] = red(a[i]))
 * The resulting array a' satisfies:
 *     a'[i] == 3*a[i] modulo Q
 *  -524287 <= a'[i] <= 536573
 */
void abstract_reduce_array(interval_t **a, uint32_t n) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = red(x);
    delete_interval(x);
  }
}

/*
 * Reduce all elements of array a twice: a'[i] = red(red(a[i]))
 * The result satisfies:
 *    a'[i] == 9*a[i] modulo Q
 *   -130 <= a'[i] <= 12413
 */
void abstract_reduce_array_twice(interval_t **a, uint32_t n) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = red_twice(x);
    delete_interval(x);
  }
}

/*
 * Convert to integers in the range [0, Q-1] after double reduction.
 * The correct function is
 *   if (x < 0) --> x + Q
 *   if (x >= Q) --> x - Q
 *   otherwise --> x
 * So the input must be in the interval [-Q, 2*Q-1] for the function to
 * produce a number in [0, Q-1].
 *
 * This function checks the pre-condition.
 */
void abstract_correct(interval_t **a, uint32_t n) {
  interval_t *x;
  uint32_t i;

  for (i=0; i<n; i++) {
    x = a[i];
    if (x->min < -12289 || x->max >= 2*12289) {
      fprintf(stderr, "Invalid input to correct: a[%"PRIu32"] in [%"PRId64", %"PRId64"]\n", i, x->min, x->max);
      fprintf(stderr, "  valid input must be in [-12289, 24577]\n");
      fprintf(stderr, "\n");
      abort();
    }
    a[i] = correct(x);
    delete_interval(x);
  }
}


/*
 * Multiply a[i] by p[i] then reduce
 * The result satisfies:
 *    a'[i] == 3 * a[i] * p[i] modulo Q
 *
 *
 * Bounds on a'[i]
 * 1) if 0 <= a[i] <= 12288 and 0 <= p[i] <= 12288 then
 *      -36864 <= a'[i] <= 12285
 *
 * 2) if -6144 <= a[i] <= 6144 and -6144 <= p[i] <= 6144 then
 *      -9216 <= a'[i] <= 21499
 *
 * In particular, if condition (2) holds, then the result is a safe input to 
 * the ntt functions.
 * 
 * mul_reduce_array16 does this in-place, with p an array of 16bit constants.
 * - a[i] * p[i] is computed using 64bit arithmetic
 *
 * mul_array builds the reduced product in array c from two 32bit arrays a and b.
 * - a[i] * b[i] is computed using 64bit arithmetic but the reduced value is
 *   converted to 32bits.
 * - to avoid overflow, we must have
 *     -8796042698752 <= a[i] * b[i] <= 8796093026303
 */
void abstract_mul_reduce_array16(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = red_scale(p[i], x);
    delete_interval(x);
  }
}

void abstract_mul_reduce_array(interval_t **c, uint32_t n, const interval_t **a, const interval_t **b) {
  uint32_t i;

  for (i=0; i<n; i++) {
    c[i] = red_mul(a[i], b[i]);
  }
}


/*
 * Multiply by c then reduce
 */
void abstract_scalar_mul_reduce_array(interval_t **a, uint32_t n, int32_t c) {
  uint32_t i;
  interval_t *x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = red_scale(c, x);
    delete_interval(x);
  }
}



/*
 * Print intervals & check overflow
 */
static void show_intervals(const char *prefix, uint32_t loop_counter, interval_t **a, uint32_t n) {
  uint32_t i;

  printf("%s[%"PRIu32"]\n", prefix, loop_counter);
  for (i=0; i<n; i++) {
    printf("     a[%"PRIu32"] in [%"PRId64", %"PRId64"]\n",  i, a[i]->min, a[i]->max);
  }
  printf("\n");

  for (i=0; i<n; i++) {
    if (a[i]->min < (int64_t) INT32_MIN || a[i]->max > (int64_t) INT32_MAX) {
      printf("    Warnning: possible overflow for a[%"PRIu32"]: bounds = [%"PRId64", %"PRId64"]\n", i, a[i]->min, a[i]->max);
    }
  }

}


/*
 * COOLEY-TUKEY/INPUT IN BIT-REVERSE ORDER/OUTPUT IN STANDARD ORDER
 */

/*
 * NTT computation
 * - input: a[0 ... n-1] in bit-reverse order
 *   with | a[i] | <= 12288
 *
 * - p: constant array of powers of omega
 *   such that p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, 4, .., n/2
 *       j=0, ..., t-1
 * - output: a contains the NTT(a) in standard order
 */
void abstract_ntt_red_ct_rev2std(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  int64_t w;

  for (t=1; t<n; t <<= 1) {
    show_intervals("ct_rev2std", t, a, n);

    /*
     * process m blocks of size t to produce m/2 blocks of size 2t
     * - m = n/t
     * - w_t for this round is omega^(n/2t) = 3 * p[t]
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      y = a[s];
      a[s + t] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // general case: j>0
    for (j=1; j<t; j++) {
      w = p[t+j];   // w_t^j/3
      for (s=j; s<n; s += t + t) {
	x = a[s + t];
	y = a[s];
	z = red_scale(w, x);
        a[s + t] = sub(y, z);
        a[s] = add(y, z);
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("ct_rev2std", t, a, n);
}

/*
 * VARIANT: p is abstract too
 */
void abstract2_ntt_red_ct_rev2std(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  const interval_t *w;

  for (t=1; t<n; t <<= 1) {
    show_intervals("ct_rev2std", t, a, n);

    /*
     * process m blocks of size t to produce m/2 blocks of size 2t
     * - m = n/t
     * - w_t for this round is omega^(n/2t) = 3 * p[t]
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      y = a[s];
      a[s + t] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // general case: j>0
    for (j=1; j<t; j++) {
      w = p[t+j];   // w_t^j/3
      for (s=j; s<n; s += t + t) {
	x = a[s + t];
	y = a[s];
	z = red_mul(x, w);
        a[s + t] = sub(y, z);
        a[s] = add(y, z);
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("ct_rev2std", t, a, n);
}


/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *
 * - output: NTT(a') in standard order, where a'[i] = a[i] * psi^i
 */
void abstract_mulntt_red_ct_rev2std(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  int64_t w;

  for (t=1; t<n; t <<= 1) {
    show_intervals("premul_ct_rev2std", t, a, n);

    /*
     * Each iteration processes n/t blocks of size t
     * to produce n/2t blocks of size 2t
     *
     * For round t:
     *   w_t = omega^(n/2t)
     *   psi_t = psi^(n/2t)
     * For a given j between 0 and t-1: 
     *   w_t,j = psi_t * w_t^j
     */
    for (j=0; j<t; j++) {
      w = p[t + j]; // w = psi_t * w_t^j * inverse(3)
      for (s=j; s<n; s += t + t) {
	x = a[s + t];
	y = a[s];
        z = red_scale(w, x);
        a[s + t] = sub(y, z);  // a[s] - x;
        a[s] = add(y, z);      // a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }    
  }

  show_intervals("premul_ct_rev2std", t, a, n);
}

void abstract2_mulntt_red_ct_rev2std(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  const interval_t *w;

  for (t=1; t<n; t <<= 1) {
    show_intervals("premul_ct_rev2std", t, a, n);

    /*
     * Each iteration processes n/t blocks of size t
     * to produce n/2t blocks of size 2t
     *
     * For round t:
     *   w_t = omega^(n/2t)
     *   psi_t = psi^(n/2t)
     * For a given j between 0 and t-1: 
     *   w_t,j = psi_t * w_t^j
     */
    for (j=0; j<t; j++) {
      w = p[t + j]; // w = psi_t * w_t^j * inverse(3)
      for (s=j; s<n; s += t + t) {
	x = a[s + t];
	y = a[s];
        z = red_mul(x, w);
        a[s + t] = sub(y, z);  // a[s] - x;
        a[s] = add(y, z);      // a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }    
  }

  show_intervals("premul_ct_rev2std", t, a, n);
}



/*
 * COOLEY-TUKEY/INPUT IN STANDARD ORDER/OUTPUT IN BIT-REVERSE ORDER
 */

/*
 * NTT computation
 * - input: a[0 ... n-1] in standard order
 * - p: constant array ow powers of omega
 *   such that p[t + j] = omega^(n/2t)^ bitrev(j) * inverse(3)
 *   for t=1, 2. 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a) in bit-reverse order
 */
void abstract_ntt_red_ct_std2rev(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  int64_t w;

  d = n;
  for (t=1; t<n; t <<= 1) {
    show_intervals("ct_std2rev", t, a, n);

    d >>= 1;
    /*
     * Invariant: d * 2t = n.
     *
     * Each iteration produces d blocks of size 2t.
     * Block i is stored at indices {i, i+d, ..., i+d*(2t-1) } in
     * bit-reverse order.
     *
     * The w_t for this round is omega^(n/2t).
     * and w_t,j is w_t^bitrev(j)
     */
    // first loop: j=0, bitrev(j) = 0
    for (s=0; s<d; s ++) {
      x = a[s + d];
      y = a[s];
      a[s + d] = sub(y, x); // a[s] - x;
      a[s] = add(y, x);     // a[s] + x;
      delete_interval(x);
      delete_interval(y);
    }
    u = 0;
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^bitrev(j)
      u += 2 * d;   // u = 2 * d * j
      for (s=u; s<u+d; s++) {
	x = a[s + d];
	y = a[s];
	z = red_scale(w, x);  // x = mul_red(a[s + d], w);
        a[s + d] = sub(y, z); // a[s + s] = a[s] - x;
        a[s] = add(y, z);     // a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("ct_std2rev", t, a, n);  
}

void abstract2_ntt_red_ct_std2rev(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  const interval_t *w;

  d = n;
  for (t=1; t<n; t <<= 1) {
    show_intervals("ct_std2rev", t, a, n);

    d >>= 1;
    /*
     * Invariant: d * 2t = n.
     *
     * Each iteration produces d blocks of size 2t.
     * Block i is stored at indices {i, i+d, ..., i+d*(2t-1) } in
     * bit-reverse order.
     *
     * The w_t for this round is omega^(n/2t).
     * and w_t,j is w_t^bitrev(j)
     */
    // first loop: j=0, bitrev(j) = 0
    for (s=0; s<d; s ++) {
      x = a[s + d];
      y = a[s];
      a[s + d] = sub(y, x); // a[s] - x;
      a[s] = add(y, x);     // a[s] + x;
      delete_interval(x);
      delete_interval(y);
    }
    u = 0;
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^bitrev(j)
      u += 2 * d;   // u = 2 * d * j
      for (s=u; s<u+d; s++) {
	x = a[s + d];
	y = a[s];
	z = red_mul(x, w);  // x = mul_red(a[s + d], w);
        a[s + d] = sub(y, z); // a[s + s] = a[s] - x;
        a[s] = add(y, z);     // a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("ct_std2rev", t, a, n);  
}


/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array ow powers of omega
 *   such that p[t + j] = psi^(n/2t) * omega^(n/2t)^ bitrev(j) * inverse(3)
 *   for t=1, 2. 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a') in reverse order where a'[i] = a[i] * psi^i
 */
void abstract_mulntt_red_ct_std2rev(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  int64_t w;

  d = n;
  for (t=1; t<n; t <<= 1) {
    show_intervals("premul_ct_std2rev", t, a, n);

    d >>= 1;
    /*
     * Invariant: d * 2t = n.
     *
     * Each iteration produces d blocks of size 2t.
     * Block i is stored at indices {i, i+d, ..., i+d*(2t-1) } in
     * bit-reverse order.
     *
     * For round t:
     *    w_t = omega^(n/2t)
     *  psi_t = psi^(n/2t)
     * For a given j between 0 and t-1
     *   w_t,j = psi_t * w_t^bitrev(j)
     */
    for (j=0, u=0; j<t; j++, u += 2*d) { // u = j * 2d
      w = p[t + j]; // psi_t * w_t^bitrev(j) * inverse(3)
      for (s=u; s<u+d; s++) {
	x = a[s + d];
	y = a[s];
	z = red_scale(w, x);
	//        x = mul_red(a[s + d], w);
	//        a[s + d] = a[s] - x;
	//        a[s] = a[s] + x;
	a[s + d] = sub(y, z);
	a[s] = add(y, z);
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("premul_ct_std2rev", t, a, n);
}

void abstract2_mulntt_red_ct_std2rev(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  const interval_t *w;

  d = n;
  for (t=1; t<n; t <<= 1) {
    show_intervals("premul_ct_std2rev", t, a, n);

    d >>= 1;
    /*
     * Invariant: d * 2t = n.
     *
     * Each iteration produces d blocks of size 2t.
     * Block i is stored at indices {i, i+d, ..., i+d*(2t-1) } in
     * bit-reverse order.
     *
     * For round t:
     *    w_t = omega^(n/2t)
     *  psi_t = psi^(n/2t)
     * For a given j between 0 and t-1
     *   w_t,j = psi_t * w_t^bitrev(j)
     */
    for (j=0, u=0; j<t; j++, u += 2*d) { // u = j * 2d
      w = p[t + j]; // psi_t * w_t^bitrev(j) * inverse(3)
      for (s=u; s<u+d; s++) {
	x = a[s + d];
	y = a[s];
	z = red_mul(x, w);
	//        x = mul_red(a[s + d], w);
	//        a[s + d] = a[s] - x;
	//        a[s] = a[s] + x;
	a[s + d] = sub(y, z);
	a[s] = add(y, z);
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("premul_ct_std2rev", t, a, n);
}


/*
 * GENTLEMAN-SANDE/INPUT IN BIT-REVERSE ORDER/OUTPUT IN STANDARD ORDER
 */

/*
 * NTT Computation
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that
 *   p[t + j] = omega^(n/2t)^rev(j) * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in standard order
 */
void abstract_ntt_red_gs_rev2std(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  int64_t w;

  t = n;
  for (d=1; d<n; d<<=1) {
    show_intervals("gs_rev2std", d, a, n);

    t >>= 1;
    /*
     * Split d blocks of size 2t into 2d blocks of size t.
     * Block i is stored at indices i+dj for j= 0 ... 2t-1, in
     * bit-reverse order.
     * w_t = omega^(n/2t) = omega^d
     */
    // first loop for j=0: w_t^rev(j) = 1
    for (s=0; s<d; s++) {
      x = a[s + d];
      y = a[s];
      //      a[s + d] = a[s] - x;
      //      a[s] = a[s] + x;
      a[s + d] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // general case, j>0, u = 2*d*j
    for (j=1, u=2*d; j<t; j++, u += 2*d) {
      w = p[t + j];  // w_t^bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
	y = a[s];
	z = sub(y, x);
	a[s + d] = red_scale(w, z);
	a[s] = add(y, x);
	//        a[s + d] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("gs_rev2std", t, a, n);
}

void abstract2_ntt_red_gs_rev2std(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  const interval_t *w;

  t = n;
  for (d=1; d<n; d<<=1) {
    show_intervals("gs_rev2std", d, a, n);

    t >>= 1;
    /*
     * Split d blocks of size 2t into 2d blocks of size t.
     * Block i is stored at indices i+dj for j= 0 ... 2t-1, in
     * bit-reverse order.
     * w_t = omega^(n/2t) = omega^d
     */
    // first loop for j=0: w_t^rev(j) = 1
    for (s=0; s<d; s++) {
      x = a[s + d];
      y = a[s];
      //      a[s + d] = a[s] - x;
      //      a[s] = a[s] + x;
      a[s + d] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // general case, j>0, u = 2*d*j
    for (j=1, u=2*d; j<t; j++, u += 2*d) {
      w = p[t + j];  // w_t^bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
	y = a[s];
	z = sub(y, x);
	a[s + d] = red_mul(z, w);
	a[s] = add(y, x);
	//        a[s + d] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("gs_rev2std", t, a, n);
}


/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^rev(j) * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multiplied by powers of psi, in standard order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void abstract_nttmul_red_gs_rev2std(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  int64_t w;

  t = n;
  for (d=1; d<n; d<<=1) {
    show_intervals("postmul_gs_rev2std", d, a, n);

    t >>= 1;
    /*
     * Split d blocks of size 2t into 2d blocks of size t.
     * Block i is stored at indices i+dj for j= 0 ... 2t-1, in
     * bit-reverse order.
     * w_t = omega^(n/2t) = omega^d
     * psi_t = psi^(n/2t)
     */
    for (j=0, u=0; j<t; j++, u += 2*d) {
      w = p[t + j];  // psi_t * w_t ^ bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
	y = a[s];
	z = sub(y, x);
	a[s + d] = red_scale(w, z);
	a[s] = add(y, x);
	//        a[s + d] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("postmul_gs_rev2std", d, a, n);
}

void abstract2_nttmul_red_gs_rev2std(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t, u, d;
  interval_t *x, *y, *z;
  const interval_t *w;

  t = n;
  for (d=1; d<n; d<<=1) {
    show_intervals("postmul_gs_rev2std", d, a, n);

    t >>= 1;
    /*
     * Split d blocks of size 2t into 2d blocks of size t.
     * Block i is stored at indices i+dj for j= 0 ... 2t-1, in
     * bit-reverse order.
     * w_t = omega^(n/2t) = omega^d
     * psi_t = psi^(n/2t)
     */
    for (j=0, u=0; j<t; j++, u += 2*d) {
      w = p[t + j];  // psi_t * w_t ^ bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
	y = a[s];
	z = sub(y, x);
	a[s + d] = red_mul(z, w);
	a[s] = add(y, x);
	//        a[s + d] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("postmul_gs_rev2std", d, a, n);
}


/*
 * GENTLEMAN-SANDE/INPUT IN STANDARD ORDER/OUTPUT IN BIT-REVERSE ORDER
 */

/*
 * NTT Computation
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in bit-reverse order
 */
void abstract_ntt_red_gs_std2rev(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  int64_t w;

  for (t = n>>1; t > 0; t >>= 1) {
    show_intervals("gs_std2rev", t, a, n);

    /*
     * Split block of size 2t into two blocks of size t
     * block i is stored at [2ti, 2ti+1, ..., 2ti + 2t - 1] in standard order
     * w_t is omega^(n/2t)
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      y = a[s];
      //      a[s + t] = a[s] - x;
      //      a[s] = a[s] + x;
      a[s + t] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // rest: j=1 to t-1
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
	y = a[s];
	z = sub(y, x);
	a[s + t] = red_scale(w, z);
	a[s] = add(y, x);
	//        a[s + t] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("gs_std2rev", t, a, n);
}

void abstract2_ntt_red_gs_std2rev(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  const interval_t *w;

  for (t = n>>1; t > 0; t >>= 1) {
    show_intervals("gs_std2rev", t, a, n);

    /*
     * Split block of size 2t into two blocks of size t
     * block i is stored at [2ti, 2ti+1, ..., 2ti + 2t - 1] in standard order
     * w_t is omega^(n/2t)
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      y = a[s];
      //      a[s + t] = a[s] - x;
      //      a[s] = a[s] + x;
      a[s + t] = sub(y, x);
      a[s] = add(y, x);
      delete_interval(x);
      delete_interval(y);
    }
    // rest: j=1 to t-1
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
	y = a[s];
	z = sub(y, x);
	a[s + t] = red_mul(z, w);
	a[s] = add(y, x);
	//        a[s + t] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }

  show_intervals("gs_std2rev", t, a, n);
}


/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multiplied by powers of psi, in reverse order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void abstract_nttmul_red_gs_std2rev(interval_t **a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  int64_t w;

  for (t = n>>1; t > 0; t >>= 1) {
    show_intervals("postmul_gs_std2rev", t, a, n);

    /*
     * Split block of size 2t into two blocks of size t
     * block i is stored at [2ti, 2ti+1, ..., 2ti + 2t - 1] in standard order
     * w_t is omega^(n/2t)
     * psi_t is psi^(n/2t)
     */
    for (j=0; j<t; j++) {
      w = p[t + j]; // psi_t * w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
	y = a[s];
	z = sub(y, x);
	a[s + t] = red_scale(w, z);
	a[s] = add(y, x);
	//        a[s + t] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }
  
  show_intervals("postmul_gs_std2rev", t, a, n);
}

void abstract2_nttmul_red_gs_std2rev(interval_t **a, uint32_t n, const interval_t **p) {
  uint32_t j, s, t;
  interval_t *x, *y, *z;
  const interval_t *w;

  for (t = n>>1; t > 0; t >>= 1) {
    show_intervals("postmul_gs_std2rev", t, a, n);

    /*
     * Split block of size 2t into two blocks of size t
     * block i is stored at [2ti, 2ti+1, ..., 2ti + 2t - 1] in standard order
     * w_t is omega^(n/2t)
     * psi_t is psi^(n/2t)
     */
    for (j=0; j<t; j++) {
      w = p[t + j]; // psi_t * w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
	y = a[s];
	z = sub(y, x);
	a[s + t] = red_mul(z, w);
	a[s] = add(y, x);
	//        a[s + t] = mul_red(a[s] - x, w);
	//        a[s] = a[s] + x;
	delete_interval(x);
	delete_interval(y);
	delete_interval(z);
      }
    }
  }
  
  show_intervals("postmul_gs_std2rev", t, a, n);
}
