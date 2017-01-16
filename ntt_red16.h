/*
 * NTT for Q=12289, n=16, using the Longa/Naehrig reduction method.
 */

#ifndef __NTT_RED16_H
#define __NTT_RED16_H

#include "ntt_red16_tables.h"
#include "ntt_red.h"

/*
 * NTT Variants: as in ntt_red.h
 * using tables from ntt16_red_tables.h
 *
 * Input: a[i] for i=0 .. 15 is expected to satisfy
 *   -21499 <= a[i] <= 21499
 *
 * The result is stored in a, it is not reduced modulo Q.
 */
// forward NTTs
static inline void ntt_red16_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 16, ntt_red16_omega_powers);
}

static inline void ntt_red16_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 16, ntt_red16_omega_powers_rev);
}

static inline void ntt_red16_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 16, ntt_red16_omega_powers_rev);
}

static inline void ntt_red16_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 16, ntt_red16_omega_powers);
}

// inverse
static inline void intt_red16_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 16, ntt_red16_inv_omega_powers);
}

static inline void intt_red16_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 16, ntt_red16_inv_omega_powers_rev);
}

static inline void intt_red16_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 16, ntt_red16_inv_omega_powers_rev);
}

static inline void intt_red16_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 16, ntt_red16_inv_omega_powers);
}

// multiplication by powers of psi then forward ntt
static inline void mulntt_red16_ct_rev2std(int32_t *a) {
  mulntt_red_ct_rev2std(a, 16, ntt_red16_mixed_powers);
}

static inline void mulntt_red16_ct_std2rev(int32_t *a) {
  mulntt_red_ct_std2rev(a, 16, ntt_red16_mixed_powers_rev);
}

// inverse ntt then multiplication by powers of psi^-1
static inline void inttmul_red16_gs_rev2std(int32_t *a) {
  nttmul_red_gs_rev2std(a, 16, ntt_red16_inv_mixed_powers_rev);
}

static inline void inttmul_red16_gs_std2rev(int32_t *a) {
  nttmul_red_gs_std2rev(a, 16, ntt_red16_inv_mixed_powers);
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
 * The input arrays must contain elements in the range [0, Q-1]
 * The result is also in that range.
 */
extern void ntt_red16_product1(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red16_product2(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red16_product3(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red16_product4(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red16_product5(int32_t *c, int32_t *a, int32_t *b);

#endif /* __NTT_RED16_H */
