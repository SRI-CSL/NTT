#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#define Q 12289
#define P 16
#define R 4

int32_t smodq(int32_t x) {
  return x % Q;
}

uint32_t umodq(uint32_t x) {
  return x % Q;
}

int32_t pmodq(int32_t x) {
  int32_t r;
  r = smodq(x);
  return (r < 0) ? r + Q : r;
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


static int64_t approx_pinv(uint32_t n) {
  return (((int64_t) 1) << n)/Q+1;
}

static int32_t approx_pdivq(int32_t x, uint32_t n) {
  return (((int64_t) x) * approx_pinv(n)) >> n;
}

static int32_t approx_pmodq(int32_t x, uint32_t n) {
  int32_t r;
  r = x - approx_pdivq(x, n) * Q;
  return (r < Q) ? r : 0;
}

// check that pmod(x, n) is between 0 and Q
static void check_approx_pmodq_bound(uint32_t n) {
  int32_t x, r;

  for (x = 0; x<INT32_MAX; x++) {
    r = approx_pmodq(x, n);
    if (r < 0 || r > Q) {
      printf("signed-approx bound (with n=%"PRIu32") fails for x = %"PRId32" (approx = %"PRId32")\n", n, x, r);
      return;
    }
  }

  r = approx_pmodq(x, n);
  if (r < 0 || r > Q) {
    printf("signed-approx bound (with n=%"PRIu32") fails for x = %"PRId32" (approx = %"PRId32")\n", n, x, r);
    return;
  }

  for (x=-1; x>INT32_MIN; x--) {
    r = approx_pmodq(x, n);
    if (r < 0 || r > Q) {
      printf("signed-approx bound (with n=%"PRIu32") fails for x = %"PRId32" (approx = %"PRId32")\n", n, x, r);
      return;
    }
  }
  
  r = approx_pmodq(x, n);
  if (r < 0 || r > Q) {
    printf("signed-approx bound (with n=%"PRIu32") fails for x = %"PRId32" (approx = %"PRId32")\n", n, x, r);
    return;
  }

  printf("Bound for signed-approx (with n=%"PRIu32") holds\n", n);
}

static void test_pmod_approx(uint32_t n) {
  int32_t x;

  for (x = 0; x<INT32_MAX; x++) {
    if (approx_pmodq(x, n) != pmodq(x)) {
      printf("signed-approx %"PRIu32" fails for %"PRId32": pmod = %"PRId32", approx = %"PRId32"\n", n, x, pmodq(x), approx_pmodq(x, n));
      return;
    }
  }

  if (approx_pmodq(x, n) != pmodq(x)) {
    printf("signed-approx %"PRIu32" fails for %"PRId32": pmod = %"PRId32", approx = %"PRId32"\n", n, x, pmodq(x), approx_pmodq(x, n));
    return;
  }

  for (x = -1; x>INT32_MIN; x--) {
    if (approx_pmodq(x, n) != pmodq(x)) {
      printf("neg: signed-approx %"PRIu32" fails for %"PRId32": pmod = %"PRId32", approx = %"PRId32"\n", n, x, pmodq(x), approx_pmodq(x, n));
      return;
    }
  }

  if (approx_pmodq(x, n) != pmodq(x)) {
    printf("signed-approx %"PRIu32" fails for %"PRId32": pmod = %"PRId32", approx = %"PRId32"\n", n, x, pmodq(x), approx_pmodq(x, n));
    return;
  }
  
  printf("signed-approx %"PRIu32" works\n", n);
  printf("checking bounds\n");
  check_approx_pmodq_bound(n);
}

static void test_all_pmod_approx(void) {
  uint32_t n;

  for (n=30; n<50; n++) {
    printf("trying signed-approx %"PRIu32", approx(1/q) = %"PRId64"\n", n, approx_pinv(n));
    test_pmod_approx(n);
    printf("\n");
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
  printf("\n");
  test_all_pmod_approx();

  return 0;
}
