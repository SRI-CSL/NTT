#include "../ntt_red_interval.h"
#include "ntt_red1024_tables.h"

#define Q 12289

/*
 * inverse NTT, CT, rev2std
 *
 * static inline void intt_red1024_ct_rev2std(int32_t *a) {
 *   ntt_red_ct_rev2std(a, 1024, ntt_red1024_inv_omega_powers);
 * }
 */

int main(void) {
  interval_t *a[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = interval(0, Q-1);
  }
  abstract_ntt_red_ct_rev2std(a, 1024, ntt_red1024_inv_omega_powers);

  return 0;
}
