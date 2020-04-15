/*
 * NTT for Q=12289 and n=256.
 */

#include "naive_ntt256.h"

/*
 * Product of two polynomials
 */
void naive_ntt256_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 256, ntt256_psi_powers, 12289);
  naive_ntt256_ct_std2rev(a);
  mul_array16_naive(b, 256, ntt256_psi_powers, 12289);
  naive_ntt256_ct_std2rev(b);
  mul_array_naive(c, 256, a, b, 12289);
  naive_intt256_ct_rev2std(c);
  mul_array16_naive(c, 256, ntt256_scaled_inv_psi_powers, 12289);
}

void naive_ntt256_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 256, ntt256_psi_powers, 12289);
  naive_ntt256_gs_std2rev(a);
  mul_array16_naive(b, 256, ntt256_psi_powers, 12289);
  naive_ntt256_gs_std2rev(b);
  mul_array_naive(c, 256, a, b, 12289);
  naive_intt256_ct_rev2std(c);
  mul_array16_naive(c, 256, ntt256_scaled_inv_psi_powers, 12289);
}

void naive_ntt256_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 256, ntt256_psi_powers, 12289);
  naive_ntt256_ct_std2rev(a);
  mul_array16_naive(b, 256, ntt256_psi_powers, 12289);
  naive_ntt256_ct_std2rev(b);
  mul_array_naive(c, 256, a, b, 12289);
  naive_intt256_gs_rev2std(c);
  mul_array16_naive(c, 256, ntt256_scaled_inv_psi_powers, 12289);
}

void naive_ntt256_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 256, ntt256_psi_powers, 12289);
  naive_ntt256_gs_std2rev(a);
  mul_array16_naive(b, 256, ntt256_psi_powers, 12289);
  naive_ntt256_gs_std2rev(b);
  mul_array_naive(c, 256, a, b, 12289);
  naive_intt256_gs_rev2std(c);
  mul_array16_naive(c, 256, ntt256_scaled_inv_psi_powers, 12289);
}


/*
 * Use combined mulntt then inttmul
 */
void naive_ntt256_product5(int32_t *c, int32_t *a, int32_t *b) {
  naive_mulntt256_ct_std2rev(a);
  naive_mulntt256_ct_std2rev(b);
  mul_array_naive(c, 256, a, b, 12289);
  naive_inttmul256_gs_rev2std(c);
  scalar_mul_array_naive(c, 256, ntt256_inv_n, 12289); // divide by n
}
