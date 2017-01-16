/*
 * NTT for Q=12289 and n=256.
 */

#include "ntt256.h"

/*
 * Product of two polynomials
 */
void ntt256_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_ct_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}

void ntt256_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_ct_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}

void ntt256_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_gs_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}

void ntt256_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_gs_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}

/*
 * Use combined mulntt then inttmul
 */
void ntt256_product5(int32_t *c, int32_t *a, int32_t *b) {
  mulntt256_ct_std2rev(a);
  mulntt256_ct_std2rev(b);
  mul_array(c, 256, a, b);
  inttmul256_gs_rev2std(c);
  scalar_mul_array(c, 256, ntt256_inv_n); // divide by n
}
