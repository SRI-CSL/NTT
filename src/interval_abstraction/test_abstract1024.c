#include "ntt_red_interval.h"
#include "ntt_red1024_tables.h"

/*
 * Test: input = a symbolic array of 16 integers.
 */
int main(void) {
  interval_t *a[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    //    a[i] = interval(0, 12288);
    a[i] = interval(-21449, 21449);
  }

  abstract_ntt_red_ct_rev2std(a, 1024, ntt_red1024_omega_powers);

  return 0;
}
