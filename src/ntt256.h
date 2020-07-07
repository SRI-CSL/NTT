/*
 * NTT for Q=12289 and n=256
 */

#ifndef __NTT256_H
#define __NTT256_H

#include "ntt256_tables.h"
#include "ntt.h"

/*
 * NTT VARIANTS
 *
 * - the input a is an array of n integers that must be between 0 and Q-1
 * - the result is stored in place
 * - the inverse transforms return a result scaled by n:
 *    we have intt(ntt(a)) = n * a
 */
// forward
static inline void ntt256_ct_rev2std(int32_t *a) {
  ntt_ct_rev2std(a, 256, ntt256_omega_powers);
}

static inline void ntt256_gs_rev2std(int32_t *a) {
  ntt_gs_rev2std(a, 256, ntt256_omega_powers_rev);
}

static inline void ntt256_ct_std2rev(int32_t *a) {
  ntt_ct_std2rev(a, 256, ntt256_omega_powers_rev);
}

static inline void ntt256_gs_std2rev(int32_t *a) {
  ntt_gs_std2rev(a, 256, ntt256_omega_powers);
}

// inverse
static inline void intt256_ct_rev2std(int32_t *a) {
  ntt_ct_rev2std(a, 256, ntt256_inv_omega_powers);
}

static inline void intt256_gs_rev2std(int32_t *a) {
  ntt_gs_rev2std(a, 256, ntt256_inv_omega_powers_rev);
}

static inline void intt256_ct_std2rev(int32_t *a) {
  ntt_ct_std2rev(a, 256, ntt256_inv_omega_powers_rev);
}

static inline void intt256_gs_std2rev(int32_t *a) {
  ntt_gs_std2rev(a, 256, ntt256_inv_omega_powers);
}

// multiplication by powers of psi then forward ntt
static inline void mulntt256_ct_rev2std(int32_t *a) {
  mulntt_ct_rev2std(a, 256, ntt256_mixed_powers);
}

static inline void mulntt256_ct_std2rev(int32_t *a) {
  mulntt_ct_std2rev(a, 256, ntt256_mixed_powers_rev);
}

// inverse ntt then multiplication by powers of psi^-1
static inline void inttmul256_gs_rev2std(int32_t *a) {
  nttmul_gs_rev2std(a, 256, ntt256_inv_mixed_powers_rev);
}

static inline void inttmul256_gs_std2rev(int32_t *a) {
  nttmul_gs_std2rev(a, 256, ntt256_inv_mixed_powers);
}


/*
 * PRODUCTS
 */

/*
 * Input: two arrays a and b in standard order
 * Result: 
 * - the product is stored in array c, in standard order.
 * - arrays a and b are modified
 *
 * The input arrays must contain elements in the range [0 .. Q-1]
 * The result is also in that range.
 */
extern void ntt256_product1(int32_t *c, int32_t *a, int32_t *b);
extern void ntt256_product2(int32_t *c, int32_t *a, int32_t *b);
extern void ntt256_product3(int32_t *c, int32_t *a, int32_t *b);
extern void ntt256_product4(int32_t *c, int32_t *a, int32_t *b);
extern void ntt256_product5(int32_t *c, int32_t *a, int32_t *b);
extern void ntt256_product6(int32_t *c, int32_t *a, int32_t *b);

#endif /* __NTT256_H */
