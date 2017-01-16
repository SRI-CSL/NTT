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

#include "ntt.h"

#define Q 12289

/*
 * UTILITIES
 */

/*
 * Bit-reverse shuffle: generic form
 */
void bitrev_shuffle(int32_t *a, uint32_t n) {
  uint32_t i, j, k;
  int32_t x;

  assert(n > 0 && (n & (n - 1)) == 0); // n must be a power of 2

  j = n>>1;
  for (i=1; i<n-1; i++) {
    if (i < j) {
      x = a[i]; a[i] = a[j]; a[j] = x;
    }
    k = n;
    do {
      k >>= 1;
      j ^= k;
    } while ((j & k) == 0);
  }
}

/*
 * Bit-Reverse using a pre-computed table of pairs [i, j] with i<j and j = bit-reverse(i)
 * - n = number of pairs in the table
 */
void shuffle_with_table(int32_t *a, const uint16_t p[][2], uint32_t n) {
  uint32_t i, j, k;
  int32_t x;

  for (i=0; i<n; i++) {
    j = p[i][0];
    k = p[i][1];
    x = a[j]; a[j] = a[k]; a[k] = x;
  }
}


/*
 * REDUCTIONS MODULO Q
 */

/*
 * x and y are assumed to be between 0 and q-1
 */
static inline int32_t sub_mod(int32_t x, int32_t y) {
  x -= y;
  // return x < 0 ? x + Q : x;
  // return x + ((x >> 31) & Q); // x + Q * sign(x)
  return x + ((x >> 14) & Q); // shift by 14 is enough
}

static inline int32_t add_mod(int32_t x, int32_t y) {
  //  x += y;
  //  return x - Q >= 0 ? x - Q : x;
  x += y - Q;
  return x + ((x >> 14) & Q);
}

/*
 * To avoid division instructions, compilers use the equalities:
 *    x%Q = x - (x/Q) * Q
 *    x/Q = (x * K) >> k
 * for a constant K that's close to 2^k/Q. Clang/gcc use
 * k=45 and K=2863078533.
 *
 * We can pick other values for k, since x is between 0 and
 * (Q-1)^2. The following values work.
 *
 *       k |     K
 *   ----------------------
 *      41 |  178942409
 *      42 |  357884817
 *      43 |  715769634
 *      44 | 1431539267
 *      45 | 2863078533
 */
static inline uint32_t divq(int32_t x) {
  return (((uint64_t) x) * 178942409) >> 41;
}

static inline int32_t modq(int32_t x) {
  return x - divq(x) * Q;
}


/*
 * ELEMENTWISE PRODUCTS
 */

/*
 * In-place product: a[i] = a[i] * p[i]
 * - a[i] and p[i] must be between 0 and Q-1
 * - p is assumed to be a array of 16bit constants (e.g., p[i] = psi^i mod Q)
 */
void mul_array16(int32_t *a, uint32_t n, const uint16_t *p) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = modq(a[i] * p[i]);
  }
}

/*
 * Elementwise product: c[i] = a[i] * b[i]
 * - all arrays are 32bit integers of size n
 */
void mul_array(int32_t *c, uint32_t n, const int32_t *a, const int32_t *b) {
  uint32_t i;

  for (i=0; i<n; i++) {
    c[i] = modq(a[i] * b[i]);
  }
}


/*
 * PRODUCT BY A SCALAR
 */
/*
 * In-place product: a[i] = a[i] * c
 * - all elements must be in the range [0 ... q-1] and c too
 */
void scalar_mul_array(int32_t *a, uint32_t n, int32_t c) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = modq(a[i] * c);
  }
}


/*
 * COOLEY-TUKEY/INPUT IN BIT-REVERSE ORDER/OUTPUT IN STANDARD ORDER
 */

/*
 * Version 1:
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array of powers of psi: p[i] = psi^i modulo Q
 * - output: a contains the NTT(a) in standard order.
 *
 * This is the same procedure as in ntt_blzzd.
 */
void ntt_ct_rev2std_v1(int32_t *a, uint32_t n, const uint16_t *p) {
  uint32_t j, s, t, l;
  int32_t x, w;

  for (t=1, l = n; t<n; t <<= 1, l >>= 1) {
    /*
     * invariant: n = l * t 
     *
     * Each iteration processes l blocks of size t
     * to produce l/2 blocks of size 2t
     * The w_t for round t is omega^(n/2t) = psi^(n/t) = psi^l
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      a[s + t] = sub_mod(a[s], x);
      a[s] = add_mod(a[s], x);
    }
    // general case: j>0
    for (j=1; j<t; j++) {
      w = p[j*l]; // w_t^j = psi^l*j
      for (s=j; s<n; s += t + t) {
        //	x = (a[s + t] * w) % Q;
        x = modq(a[s + t] * w);
        a[s + t] = sub_mod(a[s], x);
        a[s] = add_mod(a[s], x);
      }
    }    
  }
}


/*
 * Version 2:
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array of powers of omega
 *   such that p[t + j] = omega^(n/2t)^j
 *   for t=1, 2, 4, .., n/2
 *       j=0, ..., t-1
 *   this array looks like:
 *     p[0] --> not used
 *     p[1] --> 1
 *     p[2] --> 1, omega^(n/)
 *     p[4] --> 1, omega^(n/4), omega^(2*n/2), omega^(3*n/4)
 *     p[8] --> 1, ...
 *
 * - output: a contains the NTT(a) in standard order
 */
void ntt_ct_rev2std(int32_t *a, uint32_t n, const uint16_t *p) {
  uint32_t j, s, t;
  int32_t x, w;

  for (t=1; t<n; t <<= 1) {
    /*
     * process m blocks of size t to produce m/2 blocks of size 2t
     * - m = n/t
     * - w_t for this round is omega^(n/2t) = p[t]
     */
    // first loop: j=0 so w_t^j = 1
    for (s=0; s<n; s += t + t) {
      x = a[s + t];
      a[s + t] = sub_mod(a[s], x);
      a[s] = add_mod(a[s], x);
    }
    // general case: j>0
    for (j=1; j<t; j++) {
      w = p[t+j];   // w_t^j
      for (s=j; s<n; s += t + t) {
        //	x = (a[s + t] * w) % Q;
        x = modq(a[s + t] * w);
        a[s + t] = sub_mod(a[s], x);
        a[s] = add_mod(a[s], x);
      }
    }
  }
}

/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that 
 *   p[t+j] = psi^(n/2t) * omega^(n/2t)^j
 *
 * - output: NTT(a') in standard order, where a'[i] = a[i] * psi^i
 */
void mulntt_ct_rev2std(int32_t *a, uint32_t n, const uint16_t *p) {
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
      w = p[t + j]; // w = psi_t * w_t^j
      for (s=j; s<n; s += t + t) {
        //	x = (a[s + t] * w) % Q;
        x = modq(a[s + t] * w);
        a[s + t] = sub_mod(a[s], x);
        a[s] = add_mod(a[s], x);
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
 *   such that p[t + j] = omega^(n/2t)^ bitrev(j)
 *   for t=1, 2. 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a) in bit-reverse order
 */
void ntt_ct_std2rev(int32_t *a, uint32_t n, const uint16_t *p) {
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
      a[s + d] = sub_mod(a[s], x);
      a[s] = add_mod(a[s], x);
    }
    u = 0;
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^bitrev(j)
      u += 2 * d;   // u = 2 * d * j
      for (s=u; s<u+d; s++) {
        x = modq(a[s + d] * w);
        a[s + d] = sub_mod(a[s], x);
        a[s] = add_mod(a[s], x);
      }
    }
  }
}


/*
 * Combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array ow powers of omega
 *   such that p[t + j] = psi^(n/2t) * omega^(n/2t)^ bitrev(j)
 *   for t=1, 2. 4, ..., n/2
 *   and j=0, ..., t-1.
 *
 * - output: NTT(a') in reverse order where a'[i] = a[i] * psi^i
 */
void mulntt_ct_std2rev(int32_t *a, uint32_t n, const uint16_t *p) {
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
    for (j=0, u=0; j<t; j++, u+=2*d) { // u = j * 2d
      w = p[t + j]; // psi_t * w_t^bitrev(j)
      for (s=u; s<u+d; s++) {
        x = modq(a[s + d] * w);
        a[s + d] = sub_mod(a[s], x);
        a[s] = add_mod(a[s], x);
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
 * - p: constant array such that p[t + j] = omega^(n/2t)^rev(j)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in standard order
 */
void ntt_gs_rev2std(int32_t *a, uint32_t n, const uint16_t *p) {
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
      a[s + d] = sub_mod(a[s], x); // ((a[s] - x) * w) mod q
      a[s] = add_mod(a[s], x);      
    }
    // general case, j>0, u = 2*d*j
    for (j=1, u=2*d; j<t; j++, u += 2*d) {
      w = p[t + j];  // w_t^bitrev(j)
      for (s=u; s<u+d; s++) {
        x = a[s + d];
        a[s + d] = modq(sub_mod(a[s], x) * w); // ((a[s] - x) * w) mod q
        a[s] = add_mod(a[s], x);
      }
    }
  }
}

/*
 * Version 2: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in bit-reverse order
 * - p: constant array such that p[t + j] = psi^(n/2t) * omega^(n/2t)^rev(j)
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multipied by powers of psi, in standard order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void nttmul_gs_rev2std(int32_t *a, uint32_t n, const uint16_t *p) {
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
        a[s + d] = modq(sub_mod(a[s], x) * w); // ((a[s] - x) * w) mod q
        a[s] = add_mod(a[s], x);
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
 * - p: constant array such that p[t + j] = omega^(n/2t)^j
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) in bit-reverse order
 */
void ntt_gs_std2rev(int32_t *a, uint32_t n, const uint16_t *p) {
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
      a[s + t] = sub_mod(a[s], x);
      a[s] = add_mod(a[s], x);
    }
    // rest: j=1 to t-1
    for (j=1; j<t; j++) {
      w = p[t + j]; // w_t^j
      for (s=j; s<n; s += t + t) {
        x = a[s + t];
        a[s + t] = modq(sub_mod(a[s], x) * w); // (a[s] - x) * w_t^j mod q
        a[s] = add_mod(a[s], x);
      }
    }
  }
}

/*
 * Version 2: combined NTT and product by powers of psi
 * - input: a[0 ... n-1] in standard order
 * - p: constant array such that p[t + j] = psi^(n/2t) * omega^(n/2t)^j
 *   for t=1, 2, ...., n/2
 *       j=0 ... t-1
 *
 * - output: NTT(a) multipied by powers of psi, in reverse order
 *   (i.e., array a' such that a'[i] = NTT(a)[i] * psi^i).
 */
void nttmul_gs_std2rev(int32_t *a, uint32_t n, const uint16_t *p) {
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
        a[s + t] = modq(sub_mod(a[s], x) * w); // (a[s] - x) * w
        a[s] = add_mod(a[s], x);
      }
    }
  }
}

