/*
 * NTT for Q=12289, n=1024, using the Longa/Naehrig reduction method.
 */

#include "ntt_red_asm1024.h"

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
void ntt_red1024_product1_asm(int32_t *c, int32_t *a, int32_t *b) {
  //  shift_array(a, 1024); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 1024, ntt_red1024_psi_powers);
  ntt_red1024_ct_std2rev_asm(a);
  reduce_array_asm(a, 1024);

  //  shift_array(b, 1024);
  mul_reduce_array16_asm(b, 1024, ntt_red1024_psi_powers);
  ntt_red1024_ct_std2rev_asm(b);
  reduce_array_asm(b, 1024);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 1024, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 1024);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red1024_ct_rev2std_asm(c);
  mul_reduce_array16_asm(c, 1024, ntt_red1024_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 1024); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 1024);
}

void ntt_red1024_product2_asm(int32_t *c, int32_t *a, int32_t *b) {
  //  shift_array(a, 1024); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 1024, ntt_red1024_psi_powers);
  ntt_red1024_gs_std2rev_asm(a);
  reduce_array_asm(a, 1024);

  //  shift_array(b, 1024);
  mul_reduce_array16_asm(b, 1024, ntt_red1024_psi_powers);
  ntt_red1024_gs_std2rev_asm(b);
  reduce_array_asm(b, 1024);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 1024, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 1024);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red1024_ct_rev2std_asm(c);
  mul_reduce_array16_asm(c, 1024, ntt_red1024_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 1024); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 1024);
}

void ntt_red1024_product3_asm(int32_t *c, int32_t *a, int32_t *b) {
  //  shift_array(a, 1024); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 1024, ntt_red1024_psi_powers);
  ntt_red1024_ct_std2rev_asm(a);
  reduce_array_asm(a, 1024);

  //  shift_array(b, 1024);
  mul_reduce_array16_asm(b, 1024, ntt_red1024_psi_powers);
  ntt_red1024_ct_std2rev_asm(b);
  reduce_array_asm(b, 1024);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 1024, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 1024);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red1024_gs_rev2std_asm(c);
  mul_reduce_array16_asm(c, 1024, ntt_red1024_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 1024); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 1024);
}

void ntt_red1024_product4_asm(int32_t *c, int32_t *a, int32_t *b) {
  //  shift_array(a, 1024); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16_asm(a, 1024, ntt_red1024_psi_powers);
  ntt_red1024_gs_std2rev_asm(a);
  reduce_array_asm(a, 1024);

  //  shift_array(b, 1024);
  mul_reduce_array16_asm(b, 1024, ntt_red1024_psi_powers);
  ntt_red1024_gs_std2rev_asm(b);
  reduce_array_asm(b, 1024);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array_asm(c, 1024, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 1024);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red1024_gs_rev2std_asm(c);
  mul_reduce_array16_asm(c, 1024, ntt_red1024_scaled_inv_psi_powers);
  reduce_array_twice_asm(c, 1024); // c[i] = 9 * c[i] mod Q
  correct_asm(c, 1024);
}

void ntt_red1024_product5_asm(int32_t *c, int32_t *a, int32_t *b) {
  //  shift_array(a, 1024);
  mulntt_red1024_ct_std2rev_asm(a);
  reduce_array_asm(a, 1024);

  //  shift_array(b, 1024);
  mulntt_red1024_ct_std2rev_asm(b);
  reduce_array_asm(b, 1024);

  mul_reduce_array_asm(c, 1024, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice_asm(c, 1024);  // c[i] = 9 * c[i] mod Q

  inttmul_red1024_gs_rev2std_asm(c);
  scalar_mul_reduce_array_asm(c, 1024, ntt_red1024_rescale8);
  reduce_array_twice_asm(c, 1024);
  correct_asm(c, 1024);
}
