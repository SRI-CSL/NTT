#include <stdio.h>
#include <inttypes.h>

#define Q 12289
#define P 16
#define R 4

int32_t smodq(int32_t x) {
  return x % Q;
}

uint32_t umodq(uint32_t x) {
  return x % Q;
}

static uint32_t approx_qinv(uint32_t n) {
  return (((uint64_t)1)<< n)/Q+1;
}
static uint32_t approx_divq(uint32_t x, uint32_t n) {
  return (((uint64_t) x) * approx_qinv(n)) >> n;
}

static uint32_t approx_modq(uint32_t x, uint32_t n) {
  return x - approx_divq(x, n) * Q;
}

static void test_approx(uint32_t n) {
  uint32_t x;

  x = 0;
  do {
    if (approx_modq(x, n) != (x % Q)) {
      printf("approx %"PRIu32" fails for %"PRIu32"\n", n, x);
      return;
    }
    x ++;
    //  } while (x != 0); to test on full 32bit range
  } while (x <= (Q - 1) * (Q -1));

  printf("approx %"PRIu32" works\n", n);
}

static void test_all_approx(void) {
  uint32_t n;

  for (n=30; n<50; n++) {
    printf("trying approx %"PRIu32", approx(1/q) = %"PRIu32"\n", n, approx_qinv(n));
    test_approx(n);
  }
}

int main(void) {
  int32_t i, x, m, min;

  min = Q;
  x = R;
  for (i=0; i<100000; i++) {
    m = smodq(x);
    if (m < min) min = m;
    x = P * x + R;
  }
  
  printf("min = %"PRId32"\n", min);

  test_all_approx();

  return 0;
}
