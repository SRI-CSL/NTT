/*
 * NTT for Q=12289 and n=16.
 */

#include "naive_ntt16.h"

/*
 * Product of two polynomials
 */
void naive_ntt16_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 16, ntt16_psi_powers, 12289);
  naive_ntt16_ct_std2rev(a);
  mul_array16_naive(b, 16, ntt16_psi_powers, 12289);
  naive_ntt16_ct_std2rev(b);
  mul_array_naive(c, 16, a, b, 12289);
  naive_intt16_ct_rev2std(c);
  mul_array16_naive(c, 16, ntt16_scaled_inv_psi_powers, 12289);
}

void naive_ntt16_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 16, ntt16_psi_powers, 12289);
  naive_ntt16_gs_std2rev(a);
  mul_array16_naive(b, 16, ntt16_psi_powers, 12289);
  naive_ntt16_gs_std2rev(b);
  mul_array_naive(c, 16, a, b, 12289);
  naive_intt16_ct_rev2std(c);
  mul_array16_naive(c, 16, ntt16_scaled_inv_psi_powers, 12289);
}

void naive_ntt16_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 16, ntt16_psi_powers, 12289);
  naive_ntt16_ct_std2rev(a);
  mul_array16_naive(b, 16, ntt16_psi_powers, 12289);
  naive_ntt16_ct_std2rev(b);
  mul_array_naive(c, 16, a, b, 12289);
  naive_intt16_gs_rev2std(c);
  mul_array16_naive(c, 16, ntt16_scaled_inv_psi_powers, 12289);
}

void naive_ntt16_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 16, ntt16_psi_powers, 12289);
  naive_ntt16_gs_std2rev(a);
  mul_array16_naive(b, 16, ntt16_psi_powers, 12289);
  naive_ntt16_gs_std2rev(b);
  mul_array_naive(c, 16, a, b, 12289);
  naive_intt16_gs_rev2std(c);
  mul_array16_naive(c, 16, ntt16_scaled_inv_psi_powers, 12289);
}


/*
 * Use combined mulntt then inttmul
 */
void naive_ntt16_product5(int32_t *c, int32_t *a, int32_t *b) {
  naive_mulntt16_ct_std2rev(a);
  naive_mulntt16_ct_std2rev(b);
  mul_array_naive(c, 16, a, b, 12289);
  naive_inttmul16_gs_rev2std(c);
  scalar_mul_array_naive(c, 16, ntt16_inv_n, 12289); // divide by n
}
