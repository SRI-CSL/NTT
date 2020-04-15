/*
 * NTT for Q=12289 and n=1024.
 */

#include "naive_ntt1024.h"

/*
 * Product of two polynomials
 */
void naive_ntt1024_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_ct_std2rev(a);
  mul_array16_naive(b, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_ct_std2rev(b);
  mul_array_naive(c, 1024, a, b, 12289);
  naive_intt1024_ct_rev2std(c);
  mul_array16_naive(c, 1024, ntt1024_scaled_inv_psi_powers, 12289);
}

void naive_ntt1024_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_gs_std2rev(a);
  mul_array16_naive(b, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_gs_std2rev(b);
  mul_array_naive(c, 1024, a, b, 12289);
  naive_intt1024_ct_rev2std(c);
  mul_array16_naive(c, 1024, ntt1024_scaled_inv_psi_powers, 12289);
}

void naive_ntt1024_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_ct_std2rev(a);
  mul_array16_naive(b, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_ct_std2rev(b);
  mul_array_naive(c, 1024, a, b, 12289);
  naive_intt1024_gs_rev2std(c);
  mul_array16_naive(c, 1024, ntt1024_scaled_inv_psi_powers, 12289);
}

void naive_ntt1024_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_gs_std2rev(a);
  mul_array16_naive(b, 1024, ntt1024_psi_powers, 12289);
  naive_ntt1024_gs_std2rev(b);
  mul_array_naive(c, 1024, a, b, 12289);
  naive_intt1024_gs_rev2std(c);
  mul_array16_naive(c, 1024, ntt1024_scaled_inv_psi_powers, 12289);
}


/*
 * Use combined mulntt then inttmul
 */
void naive_ntt1024_product5(int32_t *c, int32_t *a, int32_t *b) {
  naive_mulntt1024_ct_std2rev(a);
  naive_mulntt1024_ct_std2rev(b);
  mul_array_naive(c, 1024, a, b, 12289);
  naive_inttmul1024_gs_rev2std(c);
  scalar_mul_array_naive(c, 1024, ntt1024_inv_n, 12289); // divide by n
}
