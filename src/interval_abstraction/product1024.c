/*
 * Product of two polynomials
 * Q=12289, n=1024, using the Longa/Naehrig reduction method.
 */

#include <stdio.h>
#include <inttypes.h>

#include "ntt_red_interval.h"
#include "ntt_red1024_tables.h"

static void show(const char *where, const char *name, uint32_t n, interval_t **a) {
  uint32_t i;

  printf("----\n%s\n", where);
  for (i=0; i<n; i++) {
    printf("     %s[%"PRIu32"] in [%"PRId64", %"PRId64"]\n", name, i, a[i]->min, a[i]->max);
  }
  printf("----\n\n");
}

static void abstract_ntt_red1024_ct_std2rev(interval_t **a) {
  abstract_ntt_red_ct_rev2std(a, 1024, ntt_red1024_omega_powers);
}

static void abstract_intt_red1024_ct_rev2std(interval_t **a) {
  abstract_ntt_red_ct_rev2std(a, 1024, ntt_red1024_inv_omega_powers);
}

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
void abstract_ntt_red1024_product1(interval_t **c, interval_t **a, interval_t **b) {
  show("bounds on input a", "a", 1024, a);
  abstract_mul_reduce_array16(a, 1024, ntt_red1024_psi_powers);
  show("after mul_reduce_array16(a, 1024 psi_powers)", "a", 1024, a);
  abstract_ntt_red1024_ct_std2rev(a);
  show("after ntt_red_ct_std2rev(a)", "a", 1024, a);
  abstract_reduce_array(a, 1024);
  show("after_reduce_array(a, 1024)", "a", 1024, a);

  show("bounds on input b", "b", 1024, b);
  abstract_mul_reduce_array16(b, 1024, ntt_red1024_psi_powers);
  show("after mul_reduce_array16(b, 1024 psi_powers)", "b", 1024, b);
  abstract_ntt_red1024_ct_std2rev(b);
  show("after ntt_red_ct_std2rev(b)", "b", 1024, b);
  abstract_reduce_array(b, 1024);
  show("after_reduce_array(b, 1024)", "b", 1024, b);
  
  // at this point:
  // a = NTT(a) * 3, -524287 <= a[i] <= 536573
  // b = NTT(b) * 3, -524287 <= b[i] <= 536573
  abstract_mul_reduce_array(c, 1024, (const interval_t **) a, (const interval_t **) b); // c[i] = 3 * a[i] * b[i] 
  show("after mul_reduce_array(c, 1024, a, b)", "c", 1024, c);
  //  abstract_reduce_array_twice(c, 1024);  // c[i] = 9 * c[i] mod Q
  //  show("after reduce_array_twice(c, 1024)", "c", 1024, c);
  //  abstract_reduce_array(c, 1024);  // c[i] = 9 * c[i] mod Q
  //  show("after reduce_array(c, 1024)", "c", 1024, c);

  // we have: -130 <= c[i] <= 12413
  abstract_intt_red1024_ct_rev2std(c);
  show("after intt_red1024_ct_rev2std(c)", "c", 1024, c);
  abstract_mul_reduce_array16(c, 1024, ntt_red1024_scaled_inv_psi_powers);
  show("after mul_reduce_array16(c, 1024, scaled_inv_psi_powers))", "c", 1024, c);

  abstract_reduce_array_twice(c, 1024); // c[i] = 9 * c[i] mod Q
  show("after reduce_array_twice(c, 1024)", "c", 1024, c);

  // pre-condition for correct
  abstract_correct(c, 1024);
  show("after final correct(c, 1024)", "c", 1024, c);
}


int main(void) {
  interval_t *a[1024];
  interval_t *b[1024];
  interval_t *c[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = interval(0, 12288);
    b[i] = interval(0, 12288);
  }
  abstract_ntt_red1024_product1(c, a, b);
  
  return 0;
}
