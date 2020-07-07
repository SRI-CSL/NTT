#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 12289 is 3 * 2^12 + 1
 * MASK = 2^12 - 1
 */
#define Q 12289
#define K 3
#define MASK 4095

#define MAXABS 715827882
#define MAXABS2 (((int64_t) 1) << 43)

static int64_t red(int64_t x) {
  return (3 * (x & 4095)) - (x >> 12);
}

#if 0
static void test(int64_t x, int64_t w) {
  int64_t r;

  r = red(x * w);
  if (r > INT32_MAX || r < INT32_MIN) {
    printf("  32bit overflow detected for x = %"PRId64", w = %"PRId64"\n", x, w);
    printf("  red(x * w) = %"PRId64"\n", r);
    if (-MAXABS <= x && x <= MAXABS) {
      printf("---> x is within limits\n");
      fflush(stdout);
      exit(1);
    }
  }
}

// return true on overflow
static bool test2(int64_t x) {
  int64_t r;

  r = red(x);
  if (r > INT32_MAX || r < INT32_MIN) {
    printf("  32bit overflow detected for x = %"PRId64"\n", x);
    printf("  red(x) = %"PRId64"\n", r);
    return true;
  }
  return false;
}
#endif

static int64_t lower_bound, upper_bound;

static bool test_overflow(int64_t x) {
  int64_t r;
  r = red(x);
  return r>INT32_MAX || r<INT32_MIN;
}

// check for overflow in the interval [4096*x, 4096x+4095]
static bool test_overflow2(int64_t x) {
  int64_t w;

  x *= 4096;
  for (w=0; w<4096; w++) {
    if (test_overflow(x + w)) {
      return true;
    }
  }
  return false;
}

static void find_lower_bound(void) {
  int64_t l, h, x;

  l = INT64_MIN/4096;
  h = -1;
  do {
    x = (l + h)/2;
    assert(l < x && x <= h);
    if (test_overflow2(x)) {
      l = x;
    } else {
      h = x;
    }    
  } while (l + 1 < h);

  printf("Safe lower bound = %"PRId64"\n", 4096 * h);
  lower_bound = 4096 * h;
}

static void find_upper_bound(void) {
  int64_t l, h, x;

  l = 0;
  h = INT64_MAX/4096;
  do {
    x = (l + h)/2;
    assert(l <= x && x < h);
    if (test_overflow2(x)) {
      h = x;
    } else {
      l = x;
    }    
  } while (l + 1 < h);

  printf("Safe upper bound = %"PRId64"\n", 4096 * l + 4095);
  upper_bound = 4096 * l + 4095;
}

static void iter_bound_abs(void) {
  uint32_t i;
  int64_t b, nb;

  printf("CT updates\n");
  b = Q-1;
  for (i=0; i<20; i++) {
    printf("B%"PRIu32" = %"PRId64"\n", i, b);
    nb = (K+1) * b + Q - K;
    if (nb > INT32_MAX) {
      b = b/4096 + Q - K;
      printf("Reduction to %"PRId64"\n", b);
      nb = (K+1) * b + Q - K;
    }
    b = nb;
  }
  printf("\n");

  printf("GS updates\n");
  b = Q-1;
  for (i=0; i<20; i++) {
    printf("B%"PRIu32" = %"PRIi64"\n", i, b);
    nb = 2 * K * b + Q - K;
    if (nb > INT32_MAX) {
      b = b/4096 + Q - K;
      printf("Reduction to %"PRId64"\n", b);
      nb = 2 * K * b + Q - K;
    }
    b = nb;
  }
  printf("\n");
}


/*
 * CT: x' = x + red(w * y)
 *     y' = x - red(w * y)
 * 
 * Bound on |x'| and |y'| assuming |x| <= b and |y| <= b.
 */
static int64_t ct_bound(int64_t b) {
  int64_t delta;

  delta = (Q - K) - (b+1)/4096;
  return delta < 0 ? (K + 1) * b : (K + 1) * b + delta;  
}

/*
 * GS: x' = x + y
 *     y' = red(w * (x - y))
 *
 * Bound on |x'| and |y'| assuming |x| <= b and |y| <= b.
 */
static int64_t gs_bound(int64_t b) {
  int64_t delta, bb;

  bb = 2 * b; // |x - y| <= 2b
  delta = (Q - K) - (bb + 1)/4096;
  return delta < 0 ? K * bb : K * bb + delta;
}

static void iter_bound_abs2(void) {
  uint32_t i;
  int64_t b, nb;

  printf("CT updates\n");
  b = Q-1;
  for (i=0; i<20; i++) {
    printf("B%"PRIu32" = %"PRId64"\n", i, b);
    nb = ct_bound(b);
    if (nb > INT32_MAX) {
      b = b/4096 + Q - K;
      printf("Reduction to %"PRId64"\n", b);
      nb = ct_bound(b);
    }
    b = nb;
  }

  printf("\nGS updates\n");
  b = Q-1;
  for (i=0; i<20; i++) {
    printf("B%"PRIu32" = %"PRId64"\n", i, b);
    nb = gs_bound(b);
    if (nb > INT32_MAX) {
      b = b/4096 + Q - K;
      printf("Reduction to %"PRId64"\n", b);
      nb = gs_bound(b);
    }
    b = nb;
  }
}

int main(void) {
  int64_t x;
  uint64_t u;

  find_lower_bound();
  find_upper_bound();

  for (x=-10000; x < 1000000000; x++) {
    if (test_overflow(lower_bound + x)) {
      printf("Overflow for lower_bound + %"PRId64"\n", x);
    }
  }
  printf("\n");
  for (x= -1000000000; x < 10000; x++) {
    if (test_overflow(upper_bound + x)) {
      printf("Overflow for upper_bound + %"PRId64"\n", x);
    }
  }
  printf("\n");
  u = lower_bound;
  printf("Hex: lower_bound = %0llx\n", u);
  u = upper_bound;
  printf("Hex: upper_bound = %0llx\n", u);
  printf("\n");
  
  iter_bound_abs();
  iter_bound_abs2();

  return 0;
}
