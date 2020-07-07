/*
 * NTT for Q=12289 and n=16
 */

#ifndef __NTT16_H
#define __NTT16_H

#include "ntt16_tables.h"
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
static inline void ntt16_ct_rev2std(int32_t *a) {
  ntt_ct_rev2std(a, 16, ntt16_omega_powers);
}

static inline void ntt16_gs_rev2std(int32_t *a) {
  ntt_gs_rev2std(a, 16, ntt16_omega_powers_rev);
}

static inline void ntt16_ct_std2rev(int32_t *a) {
  ntt_ct_std2rev(a, 16, ntt16_omega_powers_rev);
}

static inline void ntt16_gs_std2rev(int32_t *a) {
  ntt_gs_std2rev(a, 16, ntt16_omega_powers);
}

// inverse
static inline void intt16_ct_rev2std(int32_t *a) {
  ntt_ct_rev2std(a, 16, ntt16_inv_omega_powers);
}

static inline void intt16_gs_rev2std(int32_t *a) {
  ntt_gs_rev2std(a, 16, ntt16_inv_omega_powers_rev);
}

static inline void intt16_ct_std2rev(int32_t *a) {
  ntt_ct_std2rev(a, 16, ntt16_inv_omega_powers_rev);
}

static inline void intt16_gs_std2rev(int32_t *a) {
  ntt_gs_std2rev(a, 16, ntt16_inv_omega_powers);
}

// multiplication by powers of psi then forward ntt
static inline void mulntt16_ct_rev2std(int32_t *a) {
  mulntt_ct_rev2std(a, 16, ntt16_mixed_powers);
}

static inline void mulntt16_ct_std2rev(int32_t *a) {
  mulntt_ct_std2rev(a, 16, ntt16_mixed_powers_rev);
}

// inverse ntt then multiplication by powers of psi^-1
static inline void inttmul16_gs_rev2std(int32_t *a) {
  nttmul_gs_rev2std(a, 16, ntt16_inv_mixed_powers_rev);
}

static inline void inttmul16_gs_std2rev(int32_t *a) {
  nttmul_gs_std2rev(a, 16, ntt16_inv_mixed_powers);
}


/*
 * PRODUCTS
 */

/*
 * Input: two arrays a and b in standard order
 *
 * Result: 
 * - the product is stored in array c, in standard order.
 * - arrays a and b are modified
 *
 * The input arrays must contain elements in the range [0 .. Q-1]
 * The result is also in that range.
 *
 * The first four variants have the following form:
 * - multiply a and b by powers of psi
 * - compute NNT(a) and NTT(b) using a std2rev variant
 * - c = elementwise product of NTT(a) and NTT(b)
 * - compute INTT(c) usign a rev2std variant
 * - multiply the result by n^(-1) * powers of psi^(-1)
 * There are two choices for the NTT and INTT functions:
 * - NTT:  either ntt_ct_std2rev or ntt_gs_std2rev
 * - INTT: either intt_ct_rev2std or intt_gs_reg2std
 *
 * Product5 uses the combined mul/ntt variants:
 * - compute MULNTT(a) and MULNTT(b) using mulntt_ct_std2rev
 * - c = elementwise product
 * - compute INTTMUL(c) using inttmul_gs_rev2std
 * - multiply the result by n^(-1)
 */
extern void ntt16_product1(int32_t *c, int32_t *a, int32_t *b);
extern void ntt16_product2(int32_t *c, int32_t *a, int32_t *b);
extern void ntt16_product3(int32_t *c, int32_t *a, int32_t *b);
extern void ntt16_product4(int32_t *c, int32_t *a, int32_t *b);

extern void ntt16_product5(int32_t *c, int32_t *a, int32_t *b);

#endif /* __NTT16_H */
