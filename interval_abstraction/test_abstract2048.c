#include "ntt_red_interval.h"
#include "ntt_red2048_tables.h"

/*
 * Test: input = a symbolic array of 16 integers.
 */
int main(void) {
  interval_t *a[2048];
  uint32_t i;

  for (i=0; i<2048; i++) {
    //    a[i] = interval(0, 12288);
    a[i] = interval(-21449, 21449);
  }

  abstract_ntt_red_ct_rev2std(a, 2048, ntt_red2048_omega_powers);

  return 0;
}
