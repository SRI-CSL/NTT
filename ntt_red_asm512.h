/*
 * NTT for Q=12289, n=512, using the Longa/Naehrig reduction method.
 */

#ifndef __NTT_RED_ASM512_H
#define __NTT_RED_ASM512_H

#include "ntt_red512_tables.h"
#include "ntt_asm.h"

/*
 * NTT Variants: as in ntt_red.h
 * using tables from ntt512_red_tables.h
 *
 * Input: a[i] for i=0 .. 15 is expected to satisfy
 *   -21499 <= a[i] <= 21499
 *
 * The result is stored in a, it is not reduced modulo Q.
 */
// forward NTTs
static inline void ntt_red512_ct_rev2std_asm(int32_t *a) {
  ntt_red_ct_rev2std_asm(a, 512, ntt_red512_omega_powers);
}

static inline void ntt_red512_gs_rev2std_asm(int32_t *a) {
  ntt_red_gs_rev2std_asm(a, 512, ntt_red512_omega_powers_rev);
}

static inline void ntt_red512_ct_std2rev_asm(int32_t *a) {
  ntt_red_ct_std2rev_asm(a, 512, ntt_red512_omega_powers_rev);
}

static inline void ntt_red512_gs_std2rev_asm(int32_t *a) {
  ntt_red_gs_std2rev_asm(a, 512, ntt_red512_omega_powers);
}

// inverse
static inline void intt_red512_ct_rev2std_asm(int32_t *a) {
  ntt_red_ct_rev2std_asm(a, 512, ntt_red512_inv_omega_powers);
}

static inline void intt_red512_gs_rev2std_asm(int32_t *a) {
  ntt_red_gs_rev2std_asm(a, 512, ntt_red512_inv_omega_powers_rev);
}

static inline void intt_red512_ct_std2rev_asm(int32_t *a) {
  ntt_red_ct_std2rev_asm(a, 512, ntt_red512_inv_omega_powers_rev);
}

static inline void intt_red512_gs_std2rev_asm(int32_t *a) {
  ntt_red_gs_std2rev_asm(a, 512, ntt_red512_inv_omega_powers);
}

// multiplication by powers of psi then forward ntt
static inline void mulntt_red512_ct_rev2std_asm(int32_t *a) {
  mulntt_red_ct_rev2std_asm(a, 512, ntt_red512_mixed_powers);
}

static inline void mulntt_red512_ct_std2rev_asm(int32_t *a) {
  mulntt_red_ct_std2rev_asm(a, 512, ntt_red512_mixed_powers_rev);
}

// inverse ntt then multiplication by powers of psi^-1
static inline void inttmul_red512_gs_rev2std_asm(int32_t *a) {
  nttmul_red_gs_rev2std_asm(a, 512, ntt_red512_inv_mixed_powers_rev);
}

static inline void inttmul_red512_gs_std2rev_asm(int32_t *a) {
  nttmul_red_gs_std2rev_asm(a, 512, ntt_red512_inv_mixed_powers);
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
extern void ntt_red512_product1_asm(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red512_product2_asm(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red512_product3_asm(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red512_product4_asm(int32_t *c, int32_t *a, int32_t *b);
extern void ntt_red512_product5_asm(int32_t *c, int32_t *a, int32_t *b);

#endif /* __NTT_RED_ASM512_H */
