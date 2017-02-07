/*
 * BD: variant implementations of NTT and Inverse NTT.
 *
 * All variants are specialized to Q=12289
 * In all variants, we assume: omega is a primitive n-th
 * root of unity and psi^2 = omega.
 */

#include <assert.h>

#if 0
#include <stdio.h>
#include <inttypes.h>
#endif

#include "ntt_red.h"

/*
 * Reduction: q = 2^m*k + 1
 * - k is small,
 * - red(x) = k * (x & mask) - (x >> m)
 *   where mask = (2^m - 1).
 */
#define Q 12289
#define K 3
#define M 12
#define MASK 4095


/*
 * Longa & Naehrig reduction
 */
// single reduction
static int32_t red(int32_t x) {
  return 3 * (x & 4095) - (x >> 12);
}

// reduction of x * y using 64 bit arithmetic
static int32_t mul_red(int32_t x, int32_t y) {
  int64_t z;
  z = (int64_t) x * y;
  assert(-8796042698752 <= z && z <= 8796093026303);
  x = z & 4095;
  y = z >> 12;
  return 3 * x - y;
}

#if 0
// double reduction of x * y, 64bit arithmetic
static int32_t mul_red_twice(int32_t x, int32_t y) {
  int64_t z;
  int32_t u;

  z = (int64_t) x * y;
  x = z & 4095;
  y = (z >> 12) & 4095;
  u = z >> 24;

  return 9 * x - 3 * y + u;
}
#endif


/*
 * NORMALIZATION
 */

/*
 * The ntt_red functions produce 32bit coefficients
 * This function reduces all coefficients to integers in [0 .. q-1]
 */
void normalize(int32_t *a, uint32_t n) {
  uint32_t i;
  int32_t x;

  for (i=0; i<n; i++) {
    x = a[i] % Q;
    if (x < 0) x += Q;
    assert(0 <= x && x < Q);
    a[i] = x;
  }
}

/*
 * Same thing but also multiply all coefficients by inverse(3).
 */
void normalize_inv3(int32_t *a, uint32_t n) {
  uint32_t i;
  int32_t x;

  for (i=0; i<n; i++) {
    x = ((int64_t) a[i] * 8193) % Q;
    if (x < 0) x += Q;
    assert(0 <= x && x < Q);
    a[i] = x;
  }
}


/*
 * Shift representation: from [0 .. Q-1] to [-(Q-1)/2 .. +(Q-1)/2]
 */
void shift_array(int32_t *a, uint32_t n) {
  uint32_t i;
  int32_t x;

  for (i=0; i<n; i++) {
    x = a[i];
    a[i] = (x > (Q-1)/2) ? x - Q : x;
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
void reduce_array(int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = red(a[i]);
  }
}

/*
 * Reduce all elements of array a twice: a'[i] = red(red(a[i]))
 * The result satisfies:
 *    a'[i] == 9*a[i] modulo Q
 *   -130 <= a'[i] <= 12413
 */
void reduce_array_twice(int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = red(red(a[i]));
  }
}

/*
 * Convert to integers in the range [0, Q-1] after double reduction.
 */
void correct(int32_t *a, uint32_t n) {
  uint32_t i;
  int32_t x;

  for (i=0; i<n; i++) {
    x = a[i];
#if 1
    x += ((x >> 16) & Q);
    x -= Q;
    x += ((x >> 16) & Q);
#else
    if (x < 0) {
      x += Q;
    } else if (x > Q) {
      x -= Q;
    }
#endif
    a[i] = x;
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
void mul_reduce_array16(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = mul_red(a[i], p[i]);
  }
}

void mul_reduce_array(int32_t *c, uint32_t n, const int32_t *a, const int32_t *b) {
  uint32_t i;

  for (i=0; i<n; i++) {
    c[i] = mul_red(a[i], b[i]);
  }
}


/*
 * Multiply by c then reduce
 */
void scalar_mul_reduce_array(int32_t *a, uint32_t n, int32_t c) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = mul_red(a[i], c);
  }
}




/*
 * COOLEY-TUKEY/INPUT IN BIT-REVERSE ORDER/OUTPUT IN STANDARD ORDER
 */

/*
 * Version 1:
 * - input: a[0 ... n-1] in bit-reverse order
 *   with | a[i] | <= 12288
 *
 * - p: constant array of powers of omega
 *   such that p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, 4, .., n/2
 *       j=0, ..., t-1

 * - output: a contains the NTT(a) in standard order
 */
void ntt_red_ct_rev2std(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  int32_t x, w;

  for (t=1; t<n; t <<= 1) {
    /*
     * process m blocks of size t to produce m/2 blocks of size 2t
     * - m = n/t
     * - w_t for this round is omega^(n/2t) = 3 * p[t]
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      a[s + t] = a[s] - x;
      a[s] = a[s] + x;
    }
    // general case: j>0
    for (j=1; j<t; j++) {
      w = p[t+j];   // w_t^j/3
      for (s=j; s<n; s += t + t) {
        x = mul_red(a[s + t], w);
        a[s + t] = a[s] - x;
        a[s] = a[s] + x;
      }
    }
  }
}

/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *
 * - output: NTT(a') in standard order, where a'[i] = a[i] * psi^i
 */
void mulntt_red_ct_rev2std(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  int32_t x, w;

  for (t=1; t<n; t <<= 1) {
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
        x = mul_red(a[s + t], w);
        a[s + t] = a[s] - x;
        a[s] = a[s] + x;
      }
    }    
  }
}


/*
 * COOLEY-TUKEY/INPUT IN STANDARD ORDER/OUTPUT IN BIT-REVERSE ORDER
 */

/*
 * Version 4:
 * - input: a[0 ... n-1] in standard order
 * - p: constant array ow powers of omega
 *   such that p[t + j] = omega^(n/2t)^ bitrev(j) * inverse(3)
 *   for t=1, 2. 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a) in bit-reverse order
 */
void ntt_red_ct_std2rev(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  int32_t x, w;

  d = n;
  for (t=1; t<n; t <<= 1) {
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
      a[s + d] = a[s] - x;
      a[s] = a[s] + x;
    }
    u = 0;
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^bitrev(j)
      u += 2 * d;   // u = 2 * d * j
      for (s=u; s<u+d; s++) {
        // x = modq(a[s + d] * w);
        // a[s + d] = sub_mod(a[s], x);
        // a[s] = add_mod(a[s], x);
        x = mul_red(a[s + d], w);
        a[s + d] = a[s] - x;
        a[s] = a[s] + x;
      }
    }
  }
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
void mulntt_red_ct_std2rev(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  int32_t x, w;

  d = n;
  for (t=1; t<n; t <<= 1) {
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
        // x = modq(a[s + d] * w);
        // a[s + d] = sub_mod(a[s], x);
        // a[s] = add_mod(a[s], x);
        x = mul_red(a[s + d], w);
        a[s + d] = a[s] - x;
        a[s] = a[s] + x;
      }
    }
  }
}


/*
 * GENTLEMAN-SANDE/INPUT IN BIT-REVERSE ORDER/OUTPUT IN STANDARD ORDER
 */

/*
 * Version 1:
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that
 *   p[t + j] = omega^(n/2t)^rev(j) * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in standard order
 */
void ntt_red_gs_rev2std(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  int32_t w, x;

  t = n;
  for (d=1; d<n; d<<=1) {
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
      // a[s + d] = sub_mod(a[s], x);
      // a[s] = add_mod(a[s], x);      
      a[s + d] = a[s] - x;
      a[s] = a[s] + x;
    }
    // general case, j>0, u = 2*d*j
    for (j=1, u=2*d; j<t; j++, u += 2*d) {
      w = p[t + j];  // w_t^bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
        // a[s + d] = modq(sub_mod(a[s], x) * w);
        // a[s] = add_mod(a[s], x);
        a[s + d] = mul_red(a[s] - x, w);
        a[s] = a[s] + x;
      }
    }
  }
}

/*
 * Version 2: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^rev(j) * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multiplied by powers of psi, in standard order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void nttmul_red_gs_rev2std(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t, u, d;
  int32_t w, x;

  t = n;
  for (d=1; d<n; d<<=1) {
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
        // a[s + d] = modq(sub_mod(a[s], x) * w);
        // a[s] = add_mod(a[s], x);
        a[s + d] = mul_red(a[s] - x, w);
        a[s] = a[s] + x;
      }
    }
  }
}


/*
 * GENTLEMAN-SANDE/INPUT IN STANDARD ORDER/OUTPUT IN BIT-REVERSE ORDER
 */

/*
 * Version 1
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in bit-reverse order
 */
void ntt_red_gs_std2rev(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  int32_t w, x;

  for (t = n>>1; t > 0; t >>= 1) {
    /*
     * Split block of size 2t into two blocks of size t
     * block i is stored at [2ti, 2ti+1, ..., 2ti + 2t - 1] in standard order
     * w_t is omega^(n/2t)
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      a[s + t] = a[s] - x;
      a[s] = a[s] + x;
    }
    // rest: j=1 to t-1
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
        // a[s + t] = modq(sub_mod(a[s], x) * w);
        // a[s] = add_mod(a[s], x);
        a[s + t] = mul_red(a[s] - x, w);
        a[s] = a[s] + x;
      }
    }
  }
}

/*
 * Version 2: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that 
 *   p[t + j] = psi^(n/2t) * omega^(n/2t)^j * inverse(3)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multiplied by powers of psi, in reverse order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void nttmul_red_gs_std2rev(int32_t *a, uint32_t n, const int16_t *p) {
  uint32_t j, s, t;
  int32_t w, x;

  for (t = n>>1; t > 0; t >>= 1) {
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
        // a[s + t] = modq(sub_mod(a[s], x) * w);
        // a[s] = add_mod(a[s], x);
        a[s + t] = mul_red(a[s] - x, w);
        a[s] = a[s] + x;
      }
    }
  }
}

