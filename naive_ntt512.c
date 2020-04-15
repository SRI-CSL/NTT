/*
 * NTT for Q=12289 and n=512.
 */

#include "naive_ntt512.h"

/*
 * Product of two polynomials
 */
void naive_ntt512_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 512, ntt512_psi_powers, 12289);
  naive_ntt512_ct_std2rev(a);
  mul_array16_naive(b, 512, ntt512_psi_powers, 12289);
  naive_ntt512_ct_std2rev(b);
  mul_array_naive(c, 512, a, b, 12289);
  naive_intt512_ct_rev2std(c);
  mul_array16_naive(c, 512, ntt512_scaled_inv_psi_powers, 12289);
}

void naive_ntt512_product2(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 512, ntt512_psi_powers, 12289);
  naive_ntt512_gs_std2rev(a);
  mul_array16_naive(b, 512, ntt512_psi_powers, 12289);
  naive_ntt512_gs_std2rev(b);
  mul_array_naive(c, 512, a, b, 12289);
  naive_intt512_ct_rev2std(c);
  mul_array16_naive(c, 512, ntt512_scaled_inv_psi_powers, 12289);
}

void naive_ntt512_product3(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 512, ntt512_psi_powers, 12289);
  naive_ntt512_ct_std2rev(a);
  mul_array16_naive(b, 512, ntt512_psi_powers, 12289);
  naive_ntt512_ct_std2rev(b);
  mul_array_naive(c, 512, a, b, 12289);
  naive_intt512_gs_rev2std(c);
  mul_array16_naive(c, 512, ntt512_scaled_inv_psi_powers, 12289);
}

void naive_ntt512_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16_naive(a, 512, ntt512_psi_powers, 12289);
  naive_ntt512_gs_std2rev(a);
  mul_array16_naive(b, 512, ntt512_psi_powers, 12289);
  naive_ntt512_gs_std2rev(b);
  mul_array_naive(c, 512, a, b, 12289);
  naive_intt512_gs_rev2std(c);
  mul_array16_naive(c, 512, ntt512_scaled_inv_psi_powers, 12289);
}


/*
 * Use combined mulntt then inttmul
 */
void naive_ntt512_product5(int32_t *c, int32_t *a, int32_t *b) {
  naive_mulntt512_ct_std2rev(a);
  naive_mulntt512_ct_std2rev(b);
  mul_array_naive(c, 512, a, b, 12289);
  naive_inttmul512_gs_rev2std(c);
  scalar_mul_array_naive(c, 512, ntt512_inv_n, 12289); // divide by n
}
