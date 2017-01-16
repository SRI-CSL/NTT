/*
 * NTT for Q=12289 and n=16.
 */

#include "ntt16.h"

/*
 * Product of two polynomials
 */
void ntt16_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 16, ntt16_psi_powers);
  ntt16_ct_std2rev(a);
  mul_array16(b, 16, ntt16_psi_powers);
  ntt16_ct_std2rev(b);
  mul_array(c, 16, a, b);
  intt16_ct_rev2std(c);
  mul_array16(c, 16, ntt16_scaled_inv_psi_powers);
}

void ntt16_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 16, ntt16_psi_powers);
  ntt16_gs_std2rev(a);
  mul_array16(b, 16, ntt16_psi_powers);
  ntt16_gs_std2rev(b);
  mul_array(c, 16, a, b);
  intt16_ct_rev2std(c);
  mul_array16(c, 16, ntt16_scaled_inv_psi_powers);
}

void ntt16_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 16, ntt16_psi_powers);
  ntt16_ct_std2rev(a);
  mul_array16(b, 16, ntt16_psi_powers);
  ntt16_ct_std2rev(b);
  mul_array(c, 16, a, b);
  intt16_gs_rev2std(c);
  mul_array16(c, 16, ntt16_scaled_inv_psi_powers);
}

void ntt16_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 16, ntt16_psi_powers);
  ntt16_gs_std2rev(a);
  mul_array16(b, 16, ntt16_psi_powers);
  ntt16_gs_std2rev(b);
  mul_array(c, 16, a, b);
  intt16_gs_rev2std(c);
  mul_array16(c, 16, ntt16_scaled_inv_psi_powers);
}


/*
 * Use combined mulntt then inttmul
 */
void ntt16_product5(int32_t *c, int32_t *a, int32_t *b) {
  mulntt16_ct_std2rev(a);
  mulntt16_ct_std2rev(b);
  mul_array(c, 16, a, b);
  inttmul16_gs_rev2std(c);
  scalar_mul_array(c, 16, ntt16_inv_n); // divide by n
}
