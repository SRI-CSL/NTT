#include "ntt_red_interval.h"
#include "ntt_red512_tables.h"

/*
 * Test: input = a symbolic array of 512integers.
 */
int main(void) {
  interval_t *a[512];
  uint32_t i;

  for (i=0; i<512; i++) {
    //    a[i] = interval(0, 12288);
    a[i] = interval(-21449, 21449);
  }

  abstract_ntt_red_ct_rev2std(a, 512, ntt_red512_omega_powers);

  return 0;
}
