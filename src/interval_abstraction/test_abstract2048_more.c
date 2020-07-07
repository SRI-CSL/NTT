#include "ntt_red_interval.h"


/*
 * Test: input = a symbolic array of 2048 integers.
 */
int main(void) {
  interval_t *a[2048];
  interval_t *p[2048];
  uint32_t i;

  for (i=0; i<2048; i++) {
    a[i] = interval(0, 12288);
  }

  for (i=0; i<2048; i++) {
    p[i] = interval(-6144, 6144);
  }

  // interval analysis produces no warnings here (-6144 <= p[i] <= 6144)
  abstract2_ntt_red_ct_rev2std(a, 2048, (const interval_t **) p);

  return 0;
}
