/*
 * NTT for Q=12289 and n=512.
 */

#include "ntt512.h"

/*
 * Product of two polynomials
 */
void ntt512_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 512, ntt512_psi_powers);
  ntt512_ct_std2rev(a);
  mul_array16(b, 512, ntt512_psi_powers);
  ntt512_ct_std2rev(b);
  mul_array(c, 512, a, b);
  intt512_ct_rev2std(c);
  mul_array16(c, 512, ntt512_scaled_inv_psi_powers);
}

void ntt512_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 512, ntt512_psi_powers);
  ntt512_gs_std2rev(a);
  mul_array16(b, 512, ntt512_psi_powers);
  ntt512_gs_std2rev(b);
  mul_array(c, 512, a, b);
  intt512_ct_rev2std(c);
  mul_array16(c, 512, ntt512_scaled_inv_psi_powers);
}

void ntt512_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 512, ntt512_psi_powers);
  ntt512_ct_std2rev(a);
  mul_array16(b, 512, ntt512_psi_powers);
  ntt512_ct_std2rev(b);
  mul_array(c, 512, a, b);
  intt512_gs_rev2std(c);
  mul_array16(c, 512, ntt512_scaled_inv_psi_powers);
}

void ntt512_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 512, ntt512_psi_powers);
  ntt512_gs_std2rev(a);
  mul_array16(b, 512, ntt512_psi_powers);
  ntt512_gs_std2rev(b);
  mul_array(c, 512, a, b);
  intt512_gs_rev2std(c);
  mul_array16(c, 512, ntt512_scaled_inv_psi_powers);
}

/*
 * Use combined mulntt then inttmul
 */
void ntt512_product5(int32_t *c, int32_t *a, int32_t *b) {
  mulntt512_ct_std2rev(a);
  mulntt512_ct_std2rev(b);
  mul_array(c, 512, a, b);
  inttmul512_gs_rev2std(c);
  scalar_mul_array(c, 512, ntt512_inv_n); // divide by n
}
