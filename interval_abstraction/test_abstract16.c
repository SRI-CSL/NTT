#include "ntt_red_interval.h"


static const int16_t ntt_red16_omega_powers[16] = {
        0, -4096, -4096,   493, -4096,  5444,   493,  2381,
    -4096, -1912,  5444,  -435,   493, -1378,  2381, -4337,
};



/*
 * Test: input = a symbolic array of 16 integers.
 */
int main(void) {
  interval_t *a[16];
  uint32_t i;

  for (i=0; i<16; i++) {
    a[i] = interval(0, 12288);
    //    a[i] = interval(-21499, 21499);
  }

  abstract_ntt_red_ct_rev2std(a, 16, ntt_red16_omega_powers);

  return 0;
}
