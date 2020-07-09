#include "../ntt_red_interval.h"

#define Q 12289

/*
 * forward NTT, CT, std2rev.
 *
 * static inline void ntt_red1024_ct_std2rev(int32_t *a) {
 *   ntt_red_ct_std2rev(a, 1024, ntt_red1024_omega_powers_rev);
 * }
 */

int main(void) {
  interval_t *a[1024];
  interval_t *p[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = interval(0, Q-1);
  }
  for (i=0; i<1024; i++) {
    p[i] = interval(-(Q-1)/2, (Q-1)/2);
  }

  abstract2_ntt_red_ct_std2rev(a, 1024, (const interval_t **) p);

  return 0;
}
