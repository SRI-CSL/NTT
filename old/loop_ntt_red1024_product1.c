/*
 * Loop on calls to  ntt1024_product1 (for profiling)
 */

#include "ntt_red1024.h"

int main(void) {
  int32_t a[1024], b[1024], c[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }

  for (;;) {
    ntt_red1024_product1(c, a, b);
  }

  return 0;
}
