#include "ntt_red_interval.h"

/*
 * Test: input = a symbolic array of 1024 integers.
 */
int main(void) {
  interval_t *a[1024];
  interval_t *p[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = interval(0, 12288);
  }

  for (i=0; i<1024; i++) {
    p[i] = interval(0, 12288);
  }

  // interval analysis produces warnings here (0 <= p[i] <= 12288)
  abstract2_ntt_red_ct_rev2std(a, 1024, (const interval_t **) p);

  return 0;
}
