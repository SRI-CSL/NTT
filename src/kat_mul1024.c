/*

  Tancrede: I added to the repository Harvey’s NTT (with a file
  tools/precomputation-ntt-harvey.sage to explain how the constants
  are generated), and a new test test_poly.c which uses known values
  (a[i]*b[i]=c[i]) and verifies that INTT(NTT(a[i])*NTT(b[i])) == c[i]
  with Harvey’s NTT. I believe similar tests should be possible with
  the others NTTs in the repository, although they do not output
  numbers in [0, PARAM_Q) so the test should be adapted.

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ntt1024.h"
#include "data_poly1024.h"

static void copy_poly(int32_t a[1024], const int32_t b[1024]) {
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = b[i];
  }
}

static void test_mul_from_KAT_values(void (*f)(int32_t *, int32_t *, int32_t *)) {
  int32_t ua[1024], ub[1024], uc[1024];

  for (int i = 0; i < REPETITIONS; i++) {
    copy_poly(ua, a[i]);
    copy_poly(ub, b[i]);
    f(uc, ua, ub);

    for (int j = 0; j < 1024; j++) {
      if (uc[j] != c[i][j]) {
	printf("\t Failure at round %d on coeff %d: %"PRIi32" != %"PRIi32".\n", i, j, uc[j], c[i][j]);
	exit(EXIT_FAILURE);
      }
    }
  }

  printf("\t Success after %d tests\n", REPETITIONS);
}

int main(void){
  build_kat();

  printf("Testing ntt1024_product1 (KAT values)\n");
  test_mul_from_KAT_values(ntt1024_product1);

  printf("\nTesting ntt1024_product2 (KAT values)\n");
  test_mul_from_KAT_values(ntt1024_product2);

  printf("\nTesting ntt1024_product3 (KAT values)\n");
  test_mul_from_KAT_values(ntt1024_product3);

  printf("\nTesting ntt1024_product4 (KAT values)\n");
  test_mul_from_KAT_values(ntt1024_product4);

  printf("\nTesting ntt1024_product5 (KAT values)\n");
  test_mul_from_KAT_values(ntt1024_product5);

  return 0;
}
