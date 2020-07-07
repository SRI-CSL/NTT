/*
 * NTT for Q=12289, n=16, using the Longa/Naehrig reduction method.
 */

#include "ntt_red16.h"

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
void ntt_red16_product1(int32_t *c, int32_t *a, int32_t *b) {
  shift_array(a, 16); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16(a, 16, ntt_red16_psi_powers);
  ntt_red16_ct_std2rev(a);
  reduce_array(a, 16);

  shift_array(b, 16);
  mul_reduce_array16(b, 16, ntt_red16_psi_powers);
  ntt_red16_ct_std2rev(b);
  reduce_array(b, 16);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array(c, 16, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice(c, 16);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red16_ct_rev2std(c);
  mul_reduce_array16(c, 16, ntt_red16_scaled_inv_psi_powers);
  reduce_array_twice(c, 16); // c[i] = 9 * c[i] mod Q
  correct(c, 16);
}

void ntt_red16_product2(int32_t *c, int32_t *a, int32_t *b) {
  shift_array(a, 16); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16(a, 16, ntt_red16_psi_powers);
  ntt_red16_gs_std2rev(a);
  reduce_array(a, 16);

  shift_array(b, 16);
  mul_reduce_array16(b, 16, ntt_red16_psi_powers);
  ntt_red16_gs_std2rev(b);
  reduce_array(b, 16);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array(c, 16, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice(c, 16);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red16_ct_rev2std(c);
  mul_reduce_array16(c, 16, ntt_red16_scaled_inv_psi_powers);
  reduce_array_twice(c, 16); // c[i] = 9 * c[i] mod Q
  correct(c, 16);
}

void ntt_red16_product3(int32_t *c, int32_t *a, int32_t *b) {
  shift_array(a, 16); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16(a, 16, ntt_red16_psi_powers);
  ntt_red16_ct_std2rev(a);
  reduce_array(a, 16);

  shift_array(b, 16);
  mul_reduce_array16(b, 16, ntt_red16_psi_powers);
  ntt_red16_ct_std2rev(b);
  reduce_array(b, 16);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array(c, 16, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice(c, 16);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red16_gs_rev2std(c);
  mul_reduce_array16(c, 16, ntt_red16_scaled_inv_psi_powers);
  reduce_array_twice(c, 16); // c[i] = 9 * c[i] mod Q
  correct(c, 16);
}

void ntt_red16_product4(int32_t *c, int32_t *a, int32_t *b) {
  shift_array(a, 16); // convert to [-(Q-1)/2, (Q-1)/2]
  mul_reduce_array16(a, 16, ntt_red16_psi_powers);
  ntt_red16_gs_std2rev(a);
  reduce_array(a, 16);

  shift_array(b, 16);
  mul_reduce_array16(b, 16, ntt_red16_psi_powers);
  ntt_red16_gs_std2rev(b);
  reduce_array(b, 16);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  mul_reduce_array(c, 16, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice(c, 16);  // c[i] = 9 * c[i] mod Q

  // we have: -130 <= c[i] <= 12413
  intt_red16_gs_rev2std(c);
  mul_reduce_array16(c, 16, ntt_red16_scaled_inv_psi_powers);
  reduce_array_twice(c, 16); // c[i] = 9 * c[i] mod Q
  correct(c, 16);
}

void ntt_red16_product5(int32_t *c, int32_t *a, int32_t *b) {
  shift_array(a, 16);
  mulntt_red16_ct_std2rev(a);
  reduce_array(a, 16);

  shift_array(b, 16);
  mulntt_red16_ct_std2rev(b);
  reduce_array(b, 16);

  mul_reduce_array(c, 16, a, b); // c[i] = 3 * a[i] * b[i] 
  reduce_array_twice(c, 16);  // c[i] = 9 * c[i] mod Q

  inttmul_red16_gs_rev2std(c);
  scalar_mul_reduce_array(c, 16, ntt_red16_rescale8);
  reduce_array_twice(c, 16);
  correct(c, 16);
}
