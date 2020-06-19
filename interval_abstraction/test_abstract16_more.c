#include "ntt_red_interval.h"


/*
 * Test: input = a symbolic array of 16 integers.
 */
int main(void) {
  interval_t *a[16];
  interval_t *p[16];
  uint32_t i;

  for (i=0; i<16; i++) {
    a[i] = interval(-21499, 21499);
    //    a[i] = interval(0, 12288);
  }

  for (i=0; i<16; i++) {
    p[i] = interval(-6144, 6144);
  }

  abstract2_ntt_red_ct_rev2std(a, 16, (const interval_t **) p);

  return 0;
}
