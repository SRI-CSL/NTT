/*
 * NTT for Q=12289 and n=1024.
 */

#include "ntt1024.h"

/*
 * Product of two polynomials
 */
void ntt1024_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 1024, ntt1024_psi_powers);
  ntt1024_ct_std2rev(a);
  mul_array16(b, 1024, ntt1024_psi_powers);
  ntt1024_ct_std2rev(b);
  mul_array(c, 1024, a, b);
  intt1024_ct_rev2std(c);
  mul_array16(c, 1024, ntt1024_scaled_inv_psi_powers);
}

void ntt1024_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 1024, ntt1024_psi_powers);
  ntt1024_gs_std2rev(a);
  mul_array16(b, 1024, ntt1024_psi_powers);
  ntt1024_gs_std2rev(b);
  mul_array(c, 1024, a, b);
  intt1024_ct_rev2std(c);
  mul_array16(c, 1024, ntt1024_scaled_inv_psi_powers);
}

void ntt1024_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 1024, ntt1024_psi_powers);
  ntt1024_ct_std2rev(a);
  mul_array16(b, 1024, ntt1024_psi_powers);
  ntt1024_ct_std2rev(b);
  mul_array(c, 1024, a, b);
  intt1024_gs_rev2std(c);
  mul_array16(c, 1024, ntt1024_scaled_inv_psi_powers);
}

void ntt1024_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 1024, ntt1024_psi_powers);
  ntt1024_gs_std2rev(a);
  mul_array16(b, 1024, ntt1024_psi_powers);
  ntt1024_gs_std2rev(b);
  mul_array(c, 1024, a, b);
  intt1024_gs_rev2std(c);
  mul_array16(c, 1024, ntt1024_scaled_inv_psi_powers);
}

/*
 * Use combined mulntt then inttmul
 */
void ntt1024_product5(int32_t *c, int32_t *a, int32_t *b) {
  mulntt1024_ct_std2rev(a);
  mulntt1024_ct_std2rev(b);
  mul_array(c, 1024, a, b);
  inttmul1024_gs_rev2std(c);
  scalar_mul_array(c, 1024, ntt1024_inv_n); // divide by n
}
