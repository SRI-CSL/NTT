/*
 * Loop on calls to  ntts256_product4 (for profiling)
 */

#include "ntt256.h"

int main(void) {
  int32_t a[256], b[256], c[256];
  uint32_t i;

  for (i=0; i<256; i++) {
    a[i] = i;
    b[i] = i;
  }

  for (;;) {
    ntt256_product4(c, a, b);
  }

  return 0;
}
