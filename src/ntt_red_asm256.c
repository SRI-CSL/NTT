/*
 * NTT for Q=12289, n=256, using the Longa/Naehrig reduction method.
 */

#include "ntt_red_asm256.h"

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
void ntt_red256_product1_asm(int32_t *c, int32_t *a, int32_t *b) {
  shift_array_asm(a, 256); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 256, ntt_red256_psi_powers);
  ntt_red256_ct_std2rev_asm(a);
  reduce_array_asm(a, 256);

  shift_array_asm(b, 256);
  mul_reduce_array16_asm(b, 256, ntt_red256_psi_powers);
  ntt_red256_ct_std2rev_asm(b);
  reduce_array_asm(b, 256);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 256, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 256);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red256_ct_rev2std_asm(c);
  mul_reduce_array16_asm(c, 256, ntt_red256_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 256); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 256);
}

void ntt_red256_product2_asm(int32_t *c, int32_t *a, int32_t *b) {
  shift_array_asm(a, 256); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 256, ntt_red256_psi_powers);
  ntt_red256_gs_std2rev_asm(a);
  reduce_array_asm(a, 256);

  shift_array_asm(b, 256);
  mul_reduce_array16_asm(b, 256, ntt_red256_psi_powers);
  ntt_red256_gs_std2rev_asm(b);
  reduce_array_asm(b, 256);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 256, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 256);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red256_ct_rev2std_asm(c);
  mul_reduce_array16_asm(c, 256, ntt_red256_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 256); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 256);
}

void ntt_red256_product3_asm(int32_t *c, int32_t *a, int32_t *b) {
  shift_array_asm(a, 256); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 256, ntt_red256_psi_powers);
  ntt_red256_ct_std2rev_asm(a);
  reduce_array_asm(a, 256);

  shift_array_asm(b, 256);
  mul_reduce_array16_asm(b, 256, ntt_red256_psi_powers);
  ntt_red256_ct_std2rev_asm(b);
  reduce_array_asm(b, 256);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 256, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 256);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red256_gs_rev2std_asm(c);
  mul_reduce_array16_asm(c, 256, ntt_red256_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 256); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 256);
}

void ntt_red256_product4_asm(int32_t *c, int32_t *a, int32_t *b) {
  shift_array_asm(a, 256); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 256, ntt_red256_psi_powers);
  ntt_red256_gs_std2rev_asm(a);
  reduce_array_asm(a, 256);

  shift_array_asm(b, 256);
  mul_reduce_array16_asm(b, 256, ntt_red256_psi_powers);
  ntt_red256_gs_std2rev_asm(b);
  reduce_array_asm(b, 256);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 256, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 256);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red256_gs_rev2std_asm(c);
  mul_reduce_array16_asm(c, 256, ntt_red256_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 256); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 256);
}

void ntt_red256_product5_asm(int32_t *c, int32_t *a, int32_t *b) {
  shift_array_asm(a, 256);
  mulntt_red256_ct_std2rev_asm(a);
  reduce_array_asm(a, 256);

  shift_array_asm(b, 256);
  mulntt_red256_ct_std2rev_asm(b);
  reduce_array_asm(b, 256);

  mul_reduce_array_asm(c, 256, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 256);  // c[i] = 9 * c[i] mod Q

  inttmul_red256_gs_rev2std_asm(c);
  scalar_mul_reduce_array_asm(c, 256, ntt_red256_rescale);
  reduce_array_twice_asm(c, 256);
  correct_asm(c, 256);
}
