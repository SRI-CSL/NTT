#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ntt_red.h"
#include "ntt_asm.h"
#include "sort.h"

#define Q 12289

/*
 * For speed measurements: counter of CPU cycles
 */
static inline uint64_t cpucycles(void) {
  uint64_t result;
  __asm__ volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax"
    : "=a" (result) ::  "%rdx");
  return result;
}

#define NTESTS 102400

static uint64_t t[NTESTS];

// Average run time
static uint64_t average_time(void) {
  uint64_t s;
  uint32_t i;

  s = 0;
  for (i=0; i<NTESTS; i++) {
    s += t[i];
  }
  return s/NTESTS;
}

// Median
static uint64_t median_time(void) {
  uint32_t i;

  sort(t, NTESTS);
  for (i=1; i<NTESTS; i++) {
    if (t[i] < t[i-1]) {
      fprintf(stderr, "BUG in sort\n");
      exit(1);
    }
  }

  return t[NTESTS/2];
}


/*
 * Print array of size n
 */
static void print_array(FILE *f, const int32_t *a, int32_t n) {
  uint32_t i, k;

  k = 0;
  for (i=0; i<n; i++) {
    if (k == 0) fprintf(f, "  ");
    fprintf(f, "%5"PRId32, a[i]);
    k ++;
    if (k == 16) {
      fprintf(f, "\n");
      k = 0;
    } else {
      fprintf(f, " ");
    }
  }
  if (k > 0) {
    fprintf(f, "\n");
  }
}

// same thing for an array of 16bit integers
static void print_array16(FILE *f, const int16_t *a, int32_t n) {
  uint32_t i, k;

  k = 0;
  for (i=0; i<n; i++) {
    if (k == 0) fprintf(f, "  ");
    fprintf(f, "%5"PRId16, a[i]);
    k ++;
    if (k == 16) {
      fprintf(f, "\n");
      k = 0;
    } else {
      fprintf(f, " ");
    }
  }
  if (k > 0) {
    fprintf(f, "\n");
  }
}

/*
 * Check equality between a and b: arrays of n elements
 */
static bool equal_arrays(const int32_t *a, const int32_t *b, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}


/*
 * Random integer in the range [-n, n-1]
 */
static int32_t random_coeff(int32_t n) {
  int32_t x;

  assert(n > 0);

  x = random() % (2 * n);
  x -= n;
  assert(-n <= x && x <= n-1);
  return x;
}

/*
 * Store random integers in a
 * - n = number of elements
 */
static void random_array(int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = random_coeff(0x40000000);
  }
}

// 16bit elements
static void random_array16(int16_t *p, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    p[i] = (int16_t) random_coeff(32768);
  }
}

/*
 * Copy a into b
 */
static void copy_array(int32_t *b, const int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    b[i] = a[i];
  }  
}


/*
 * Cross check: apply functions f and g to the same input
 * - f and g are expected to compute the same thing
 * - f is the assembler function (from ntt_asm.h) and g is the trusted oracle (from ntt_red.h)
 */
static void cross_check(const char *name, uint32_t n, void (*f)(int32_t *, uint32_t), void (*g)(int32_t *, uint32_t)) {
  int32_t a[n], b[n];
  uint32_t j;

  printf("Testing %s: n = %"PRIu32"\n", name, n);
  for (j=0; j<10000; j++) {
    random_array(a, n);
    copy_array(b, a, n);
    f(a, n);
    g(b, n);
    if (!equal_arrays(a, b, n)) {
      printf("failed on test %"PRIu32"\n", j);
      print_array(stdout, a, n);
      printf("correct result:\n");
      print_array(stdout, b, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}


/*
 * Speed test: function f(a, n)
 * - proper alignment of array a makes a difference
 */

// global buffers used for speed tests.
static int32_t a[2048] __attribute__ ((aligned(32)));
static int32_t b[2048] __attribute__ ((aligned(32)));
static int32_t d[2048] __attribute__ ((aligned(32)));
static int16_t p[2048] __attribute__ ((aligned(32)));

static void speed_test(const char *name, uint32_t n, void (*f)(int32_t *, uint32_t)) {
  uint32_t i;
  uint64_t avg, med, c;

  random_array(a, n);

  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(a, n);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();
  printf("speed test %s (n=%"PRIu32"): median = %"PRIu64", average = %"PRIu64"\n", name, n, med, avg);
}

// variant for f(a, n, p) where p = fixed array of 16bit constants
static void speed_test2(const char *name, uint32_t n, void (*f)(int32_t *, uint32_t, const int16_t *)) {
  uint32_t i;
  uint64_t avg, med, c;

  random_array(a, n);
  random_array16(p, n);

  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(a, n, p);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();
  printf("speed test %s (n=%"PRIu32"): median = %"PRIu64", average = %"PRIu64"\n", name, n, med, avg);
}

// variant for f(a, n, b, c) where b and c are input, a is output
static void speed_test3(const char *name, uint32_t n, void (*f)(int32_t *, uint32_t, const int32_t *, const int32_t *)) {
  uint32_t i;
  uint64_t avg, med, c;

  random_array(b, n);
  random_array(d, n);

  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(a, n, b, d);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();
  printf("speed test %s (n=%"PRIu32"): median = %"PRIu64", average = %"PRIu64"\n", name, n, med, avg);
}

// variant for f(a, n, c) where a is an array, c is a scalar
static void speed_test4(const char *name, uint32_t n, void (*f)(int32_t *, uint32_t, int32_t)) {
  uint32_t i;
  uint64_t avg, med, c;

  random_array(a, n);
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(a, n, 0x111);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();
  printf("speed test %s (n=%"PRIu32"): median = %"PRIu64", average = %"PRIu64"\n", name, n, med, avg);
}

/*
 * Test of the correct/correct_asm functions
 * - the input array must have elements in [-Q,2Q-1]
 */
static int32_t random_elem_for_correct(void) {
  int32_t x;

  x = random() % (3 * Q);
  x -= Q;
  assert(-Q <= x && x <= 2 * Q - 1);
  return x;
}

static void random_array_for_correct(int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = random_elem_for_correct();
  }
}

static void test_correction(uint32_t n) {
  int32_t a[n], b[n], c[n];
  uint32_t j;

  printf("Testing correct_asm: n = %"PRIu32"\n", n);
  for (j=0; j<10000; j++) {
    random_array_for_correct(a, n);
    copy_array(b, a, n);
    copy_array(c, a, n);  // keep a copy of the input
    correct_asm(a, n);
    normalize(b, n);
    if (!equal_arrays(a, b, n)) {
      printf("failed on test %"PRIu32"\n", j);
      printf("--> input:\n");
      print_array(stdout, c, n);
      printf("--> result from correct_asm:\n");
      print_array(stdout, a, n);
      printf("--> correct result:\n");
      print_array(stdout, b, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}


/*
 * Tests for mul_reduce_array
 */
static void random_arrays_for_mul_reduce16(int32_t *a, int16_t *p, uint32_t n) {
  uint32_t i;
  int32_t x, y;
  int64_t z;

  i = 0;
  while (i < n) {
    x = random_coeff(0x40000000);
    y = random_coeff(32768);
    assert(INT16_MIN <= y && y <= INT16_MAX);
    z = (int64_t) x * (int64_t) y;
    if ( -8796042698752 <= z && z <= 8796093026303) {
      // no overflow
      a[i] = x;
      p[i] = y;
      i ++;
    }
  }
}

static void random_arrays_for_mul_reduce(int32_t *a, int32_t *b, uint32_t n) {
  uint32_t i;
  int32_t x, y;
  int64_t z;

  i = 0;
  while (i < n) {
    x = random_coeff(0x40000000);
    y = random_coeff(0x40000000);
    z = (int64_t) x * (int64_t) y;
    while (z < -8796042698752 || z > 8796093026303) {
      // reduce |y| until we get a value within bounds
      y >>= 1;
      z = (int64_t) x * (int64_t) y;
    }
    a[i] = x;
    b[i] = y;
    i ++;
  }
}

static void test_mul_reduce_array16(uint32_t n) {
  int32_t a[n], b[n], c[n];
  int16_t p[n];
  uint32_t i;

  printf("Testing mul_reduce_array16_asm: n = %"PRIu32"\n", n);
  for (i=0; i<10000; i++) {
    random_arrays_for_mul_reduce16(a, p, n);
    copy_array(b, a, n);
    copy_array(c, a, n);  // keep a copy of the input
    mul_reduce_array16_asm(a, n, p);
    mul_reduce_array16(b, n, p);
    if (! equal_arrays(a, b, n)) {
      printf("failed on test %"PRIu32"\n", i);
      printf("--> input:\n");
      print_array(stdout, c, n);
      printf("--> multipliers:\n");
      print_array16(stdout, p, n);
      printf("--> result from mul_reduce_array16_asm:\n");
      print_array(stdout, a, n);
      printf("--> correct result:\n");
      print_array(stdout, b, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}

static void test_mul_reduce_array(uint32_t n) {
  int32_t a[n], b[n], c[n], d[n];
  uint32_t i;

  printf("Testing mul_reduce_array_asm: n = %"PRIu32"\n", n);
  for (i=0; i<10000; i++) {
    // b and c are input
    // a = result from asm
    // d = result from C implementation
    random_arrays_for_mul_reduce(b, c, n);
    mul_reduce_array_asm(a, n, b, c);
    mul_reduce_array(d, n, b, c);
    if (! equal_arrays(a, d, n)) {
      printf("failed on test %"PRIu32"\n", i);
      printf("--> input1:\n");
      print_array(stdout, b, n);
      printf("--> input2:\n");
      print_array(stdout, c, n);
      printf("--> result from mul_reduce_array_asm:\n");
      print_array(stdout, a, n);
      printf("--> correct result:\n");
      print_array(stdout, d, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}


/*
 * Product by a scalar
 */
// fill in array a, and return a scalar in *c
static void random_array_for_scalar_mul(int32_t *a, uint32_t n, int32_t *c) {
  uint32_t i;
  int32_t min, max, x;

  min = INT32_MAX;
  max = INT32_MIN;
  for (i=0; i<n; i++) {
    x = random_coeff(0x40000000);
    if (x < min) min = x;
    if (x > max) max = x;
    a[i] = x;
  }

  x = random_coeff(0x40000000);
  while ((int64_t) x * min < -8796042698752 || (int64_t) x * max  > 8796093026303) {
    x >>= 1;
  }
  *c = x;
}

static void test_scalar_mul_reduce_array(uint32_t n) {
  int32_t a[n], b[n], d[n];
  int32_t c;
  uint32_t i;

  printf("Testing scalar_mul_reduce_array_asm: n = %"PRIu32"\n", n);
  for (i=0; i<10000; i++) {
    random_array_for_scalar_mul(a, n, &c);
    copy_array(b, a, n);
    copy_array(d, a, n);
    scalar_mul_reduce_array_asm(a, n, c);
    scalar_mul_reduce_array(b, n, c);
    // d = input array, c = scalar
    // a = result from asm
    // b = result from C code
    if (! equal_arrays(a, b, n)) {
      printf("failed on test %"PRIu32"\n", i);
      printf("--> input:\n");
      print_array(stdout, d, n);
      printf("--> multiplier: %"PRId32"\n", c);
      printf("--> result from scalar_mul_reduce_array_asm:\n");
      print_array(stdout, a, n);
      printf("--> result from scalar_mul_reduce_array:\n");
      print_array(stdout, b, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}

/*
 * Tests
 */
static void run_tests(void) {
  uint32_t n;

  for (n=16; n<=2048; n += n) {
    cross_check("reduce_array_asm", n, reduce_array_asm, reduce_array);
    cross_check("reduce_array_twice_asm", n, reduce_array_twice_asm, reduce_array_twice);
    test_correction(n);
    test_mul_reduce_array16(n);
    test_mul_reduce_array(n);
    test_scalar_mul_reduce_array(n);
    printf("\n");
  }

  for (n=16; n<=2048; n += n) {
    speed_test("reduce_array", n, reduce_array);
    speed_test("reduce_array_twice", n, reduce_array_twice);
    speed_test("correct", n, correct);
    speed_test2("mul_reduce_array16", n, mul_reduce_array16);
    speed_test3("mul_reduce_array", n, mul_reduce_array);
    speed_test4("scalar_mul_reduce_array", n, scalar_mul_reduce_array);
    printf("\n");
    speed_test("reduce_array_asm", n, reduce_array_asm);
    speed_test("reduce_array_twice_asm", n, reduce_array_twice_asm);
    speed_test("correct_asm", n, correct_asm);
    speed_test2("mul_reduce_array16_asm", n, mul_reduce_array16_asm);
    speed_test2("mul_reduce_array16_asm2", n, mul_reduce_array16_asm2);
    speed_test3("mul_reduce_array_asm", n, mul_reduce_array_asm);
    speed_test4("scalar_mul_reduce_array_asm", n, scalar_mul_reduce_array_asm);
    printf("\n\n");
  }
}

int main(void) {
  if (avx2_supported()) {
    printf("AVX2 is supported\n\n");
    run_tests();
  } else {
    printf("AVX2 is not supported\n");
  }
  return 0;
}
