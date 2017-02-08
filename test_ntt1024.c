/*
 * Tests for ntt1024
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "bitrev1024_table.h"
#include "ntt1024.h"
#include "sort.h"


/*
 * PERFORMANCE MEASUREMENTS
 */

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
 * UTILITIES
 */

#define Q 12289


/*
 * Print array of size n
 */
static void print_array(FILE *f, int32_t *a, int32_t n) {
  uint32_t i, k;

  k = 0;
  for (i=0; i<n; i++) {
    if (k == 0) fprintf(f, "  ");
    fprintf(f, "%5"PRIu32, a[i]);
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
 * Store polynomial X^k into a
 */
static void simple_poly(int32_t *a, uint32_t k, uint32_t n) {
  uint32_t i;

  assert(k < n);

  for (i=0; i<n; i++) {
    a[i] = 0;
  }
  a[k] = 1;
}

/*
 * Random number between 0 and Q-1
 */
static int32_t random_coeff(void) {
  int32_t x;

  x = random() % Q;
  assert(0 <= x && x <= Q-1);
  return x;
}

/*
 * Store a random polynomial in a
 */
static void random_poly(int32_t *a, uint32_t n) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = random_coeff();
  }
}

/*
 * Shuffle
 */
static void ntt1024_bitrev_shuffle(int32_t *a) {
  shuffle_with_table(a, bitrev1024, BITREV1024_NPAIRS);
}


/*
 * TESTS ON POLYNOMIALS 1, X, ..., X^15
 */

/*
 * x^k modulo q
 */
static int32_t power(int32_t x, uint32_t k) {
  int32_t y;

  y = 1;
  while (k != 0) {
    if ((k & 1) != 0) {
      y = (y * x) % Q;
    }
    k >>= 1;
    x = (x * x) % Q;
  }
  return y;
}

/*
 * Check result of NTT(X^k)
 * - omega = n-th root of unity
 * - n = size
 * - a[i] should be equal to (omega^k)^i
 */
static bool check_ntt_simple(const int32_t *a, uint32_t n, uint32_t k, int32_t omega) {
  uint32_t i;
  int32_t x, y;

  y = power(omega, k);
  x = 1;  // x = y^i
  for (i=0; i<n; i++) {
    if (a[i] != x) return false;
    x = (x * y) % Q;
  }
  return true;
}

/*
 * Check result of NTT(NTT(X^k))
 * - n = size
 * - if k=0, a[0] should be n, all other elements should be 0
 * - if k/=0, then a[n-k] should be n, all other elements should be 0
 */
static bool check_ntt_ntt_simple(const int32_t *a, uint32_t n, uint32_t k) {
  uint32_t i, j;

  assert(k < n);

  j = (k == 0) ? 0 : n - k; 
  for (i=0; i<n; i++) {
    if ((i == j && a[i] != n) || (i != j && a[i] != 0)) {
      return false;
    }
  }
  return true;
}


/*
 * Call f on array a: apply shuffle to ensure input and output are in standard order.
 * - std2rev: true means that f takes input in standard order
 *           false means that f takes input in reverse order
 */
static void apply_ntt1024(int32_t *a, void (*f)(int32_t *), bool std2rev) {
  if (std2rev) {
    f(a);
    ntt1024_bitrev_shuffle(a);
  } else {
    ntt1024_bitrev_shuffle(a);
    f(a);
  }
}

/*
 * NTT of polynomials X^i:
 * - f = NTT function (in-place on an array of 1024 integers)
 * - omega = n-th root of unity
 * - std2rev:
 *   true means that f expects input in standard order
 *   false means that f expects input in reverse order
 */
static void test_simple_polys(const char *name, void (*f)(int32_t*), int32_t omega, bool std2rev) {
  int32_t a[1024];
  uint32_t n, j;

  n = 1024;
  printf("Testing %s\n", name);
  for (j=0; j<n; j++) {
    simple_poly(a, j, n);

    apply_ntt1024(a, f, std2rev);
    if (!check_ntt_simple(a, n, j, omega)) {
      printf("failed on NTT(X^%"PRIu32"):\n", j);
      print_array(stdout, a, n);
      exit(1);
    }

    apply_ntt1024(a, f, std2rev);
    if (!check_ntt_ntt_simple(a, n, j)) {
      printf("failed on NTT(NTT(X^%"PRIu32")):\n", j);
      print_array(stdout, a, n);
      printf("\n");
      exit(1);
    }
  }
  printf("all tests passed\n\n");
}


/*
 * TEST NTT + INVERSE NTT
 */
static void check_inverse(const char *fname, const char *iname, 
			  int32_t *a, void (*forward)(int32_t *), void (*inverse)(int32_t *)) {
  int32_t b[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    b[i] = a[i];
  }
  forward(b);
  inverse(b); // invntt(ntt(b)) should give back a * n
  scalar_mul_array(b, 1024, ntt1024_inv_n); // divide by n

  if (! equal_arrays(a, b, 1024)) {
    printf("failed\n");
    printf("input:\n");
    print_array(stdout, a, 1024);
    printf("output:\n");
    print_array(stdout, b, 1024);
    printf("\n");
    exit(1);
  }
  
}

static void test_forward_inverse(const char *fname, const char *iname, void (*forward)(int32_t *), void (*inverse)(int32_t *)) {
  int32_t a[1024];
  uint32_t n;

  printf("Testing inversion\n");
  printf("  forward NTT = %s\n", fname);
  printf("  inverse NTT = %s\n", iname);
  for (n=0; n<1000; n++) {
    random_poly(a, 1024);
    check_inverse(fname, iname, a, forward, inverse);
  }
  printf("all tests passed.\n\n");
}


/*
 * TEST PRODUCT
 */
// check that c = X^m, n=size
static bool equal_simple_poly(const int32_t *c, uint32_t n, uint32_t m) {
  uint32_t i, j;
  int32_t val;

  assert(m < 2*n);
  if (m >= n) {
    // X^m = - X^j;
    j = m-n;
    val = Q -1;
  } else {
    j = m;
    val = 1;
  }
  
  for (i=0; i<n; i++) {
    if ((i == j && c[i] != val) || (i != j && c[i] != 0)) {
      return false;
    }
  }

  return true;
}

// X^n * X^m
static void test_simple_product(const char *name, void (*f)(int32_t *, int32_t *, int32_t *), uint32_t n, uint32_t m) {
  int32_t a[1024], b[1024], c[1024];

  simple_poly(a, n, 1024);
  simple_poly(b, m, 1024);
  f(c, a, b);  // c := a * b
  if (! equal_simple_poly(c, 1024, n+m)) {
    printf("%s failed on X^%"PRIu32" * X^%"PRIu32"\n", name, n, m);
    printf("output:\n");
    print_array(stdout, c, 1024);
    printf("\n");
    exit(1);
  }
}

static void test_simple_products(const char *name, void (*f)(int32_t *, int32_t *, int32_t *)) {
  uint32_t n, m;

  
  printf("Testing %s\n", name);
  for (n=0; n<1024; n++) {
    for (m=0; m<1024; m++) {
      test_simple_product(name, f, n, m);
    }
  }
  printf("all tests passed.\n\n");
}

/*
 * SPEED TESTS
 */
static void speed_test(const char *name, void (*f)(int32_t *)) {
  int32_t a[1024];
  uint32_t i;
  uint64_t avg, med, c;

  printf("speed test for %s\n", name);
  for (i=0; i<1024; i++) {
    a[i] = i;
  }

  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(a);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();
  printf("median = %"PRIu64", average = %"PRIu64"\n\n", med, avg);
}


// variant for products
static void speed_test2(const char *name, void (*f)(int32_t *, int32_t *, int32_t *)) {
  int32_t a[1024], b[1024], d[1024];
  uint32_t i;
  uint64_t avg, med, c;

  printf("speed test for %s\n", name);

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }

  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    f(d, a, b);
  }
  c = cpucycles();
  for (i=0; i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i]; 
  }
  t[i] = c - t[i];

  avg = average_time();
  med = median_time();

  printf("median = %"PRIu64", average = %"PRIu64"\n\n", med, avg);
}


int main(void) {
  test_simple_polys("ntt1024_ct_rev2std", ntt1024_ct_rev2std, ntt1024_omega, false);
  test_simple_polys("ntt1024_gs_rev2std", ntt1024_gs_rev2std, ntt1024_omega, false);
  test_simple_polys("ntt1024_ct_std2rev", ntt1024_ct_std2rev, ntt1024_omega, true);
  test_simple_polys("ntt1024_gs_std2rev", ntt1024_gs_std2rev, ntt1024_omega, true);

  test_simple_polys("intt1024_ct_rev2std", intt1024_ct_rev2std, ntt1024_inv_omega, false);
  test_simple_polys("intt1024_gs_rev2std", intt1024_gs_rev2std, ntt1024_inv_omega, false);
  test_simple_polys("intt1024_ct_std2rev", intt1024_ct_std2rev, ntt1024_inv_omega, true);
  test_simple_polys("intt1024_gs_std2rev", intt1024_gs_std2rev, ntt1024_inv_omega, true);

  test_forward_inverse("ntt1024_ct_std2rev", "intt1024_ct_rev2std", ntt1024_ct_std2rev, intt1024_ct_rev2std);
  test_forward_inverse("intt1024_ct_rev2std", "ntt1024_ct_std2rev", intt1024_ct_rev2std, ntt1024_ct_std2rev);
  test_forward_inverse("ntt1024_ct_std2rev", "intt1024_gs_rev2std", ntt1024_ct_std2rev, intt1024_gs_rev2std);
  test_forward_inverse("intt1024_gs_rev2std", "ntt1024_ct_std2rev", intt1024_gs_rev2std, ntt1024_ct_std2rev);
  test_forward_inverse("ntt1024_gs_std2rev", "intt1024_gs_rev2std", ntt1024_gs_std2rev, intt1024_gs_rev2std);
  test_forward_inverse("intt1024_gs_rev2std", "ntt1024_gs_std2rev", intt1024_gs_rev2std, ntt1024_gs_std2rev);

  test_simple_products("ntt1024_product1", ntt1024_product1);
  test_simple_products("ntt1024_product2", ntt1024_product2);
  test_simple_products("ntt1024_product3", ntt1024_product3);
  test_simple_products("ntt1024_product4", ntt1024_product4);
  test_simple_products("ntt1024_product5", ntt1024_product5);
  
  speed_test("ntt1024_ct_rev2std", ntt1024_ct_rev2std);
  speed_test("ntt1024_gs_rev2std", ntt1024_gs_rev2std);
  speed_test("ntt1024_ct_std2rev", ntt1024_ct_std2rev);
  speed_test("ntt1024_gs_std2rev", ntt1024_gs_std2rev);
  printf("\n");
  speed_test("intt1024_ct_rev2std", intt1024_ct_rev2std);
  speed_test("intt1024_gs_rev2std", intt1024_gs_rev2std);
  speed_test("intt1024_ct_std2rev", intt1024_ct_std2rev);
  speed_test("intt1024_gs_std2rev", intt1024_gs_std2rev);
  printf("\n");

  speed_test2("ntt1024_product1", ntt1024_product1);
  speed_test2("ntt1024_product2", ntt1024_product2);
  speed_test2("ntt1024_product3", ntt1024_product3);
  speed_test2("ntt1024_product4", ntt1024_product4);
  speed_test2("ntt1024_product5", ntt1024_product5);
  
  return 0;
}
