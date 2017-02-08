/*
 * Basic ntt tests
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ntt.h"
#include "test_bitrev_tables.h"
#include "test_ntt_tables.h"
#include "sort.h"


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
 * BIT-REVERSE SHUFFLES
 */
static inline void bitrev_shuffle256(int32_t *a) {
  shuffle_with_table(a, bitrev256_pair, BITREV256_NPAIRS);
}

static inline void bitrev_shuffle512(int32_t *a) {
  shuffle_with_table(a, bitrev512_pair, BITREV512_NPAIRS);
}

static inline void bitrev_shuffle1024(int32_t *a) {
  shuffle_with_table(a, bitrev1024_pair, BITREV1024_NPAIRS);
}



/*
 * CHECKS
 */

#define Q 12289

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
 * NTT of polynomials X^i:
 * - n = size
 * - f = NTT function (in-place on an array of n integers)
 * - omega = n-th root of unity
 */
static void test_simple_polys(const char *name, uint32_t n, void (*f)(int32_t*), int32_t omega) {
  int32_t a[n];
  uint32_t i, j;

  printf("Testing NTT%"PRIu32" %s\n", n, name);
  for (j=0; j<n; j++) {
    for (i=0; i<n; i++) {
      a[i] = 0;
    }
    a[j] = 1;
    f(a);
    if (!check_ntt_simple(a, n, j, omega)) {
      printf("failed on NTT(X^%"PRIu32"):\n", j);
      print_array(stdout, a, n);
      exit(1);
    }

    f(a);
    if (!check_ntt_ntt_simple(a, n, j)) {
      printf("failed on NTT(NTT(X^%"PRIu32")):\n", j);
      print_array(stdout, a, n);
      printf("\n");
      exit(1);
    }
  }
  printf("all tests passed\n");
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
 * Cross check: apply functions f and g to the same input
 * - f and g are expected to compute the same thing
 * - f is the 'tested function' and g is the trusted oracle
 */
static void cross_check(const char *name, uint32_t n, void (*f)(int32_t *), void (*g)(int32_t *)) {
  int32_t a[n], b[n];
  uint32_t i, j;

  printf("Testing %s: n = %"PRIu32"\n", name, n);
  for (j=0; j<n; j++) {
    for (i=0; i<n; i++) {
      a[i] = 0;
      b[i] = 0;
    }
    a[j] = 1;
    b[j] = 1;
    f(a);
    g(b);
    if (!equal_arrays(a, b, n)) {
      printf("failed on input X^%"PRIu32":\n", j);
      print_array(stdout, a, n);
      printf("correct result:\n");
      print_array(stdout, b, n);
      exit(1);
    }
    f(a);
    g(b);
    if (! equal_arrays(a, b, n)) {
      printf("failed on f(f(X^%"PRIu32")):\n", j);
      print_array(stdout, a, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}

/*
 * Speed tests
 */
static void speed_test(const char *name, uint32_t n, void (*f)(int32_t *)) {
  int32_t a[n];
  uint32_t i;
  uint64_t avg, med, c;

  assert(n <= 12289);

  for (i=0; i<n; i++) {
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
  printf("speed test %s: median = %"PRIu64", average = %"PRIu64"\n", name, med, avg);
}


/*
 * FUNCTIONS TO TEST
 */
// Size 16
static void ntt16_v1(int32_t *a) {
  bitrev_shuffle(a, 16);
  ntt_ct_rev2std_v1(a, 16, psi_powers_ntt16_12289);
}

static void ntt16_v2(int32_t *a) {
  bitrev_shuffle(a, 16);
  ntt_ct_rev2std(a, 16, shoup_ntt16_12289);
}

static void ntt16_v3(int32_t *a) {
  bitrev_shuffle(a, 16);
  mulntt_ct_rev2std(a, 16, shoup_ntt16_12289);
}

static void ntt16_v4(int32_t *a) {
  ntt_ct_std2rev(a, 16, rev_shoup_ntt16_12289);
  bitrev_shuffle(a, 16);
}

static void ntt16_v5(int32_t *a) {
  mulntt_ct_std2rev(a, 16, rev_shoup_ntt16_12289);
  bitrev_shuffle(a, 16);
}

static void ntt16_v6(int32_t *a) {
  bitrev_shuffle(a, 16);
  ntt_gs_rev2std(a, 16, rev_shoup_ntt16_12289);
}

static void ntt16_v7(int32_t *a) {
  bitrev_shuffle(a, 16);
  nttmul_gs_rev2std(a, 16, rev_shoup_ntt16_12289);
}

static void ntt16_v8(int32_t *a) {
  ntt_gs_std2rev(a, 16, shoup_ntt16_12289);
  bitrev_shuffle(a, 16);
}

static void ntt16_v9(int32_t *a) {
  nttmul_gs_std2rev(a, 16, shoup_ntt16_12289);
  bitrev_shuffle(a, 16);
}

// Size 256
static void ntt256_v1(int32_t *a) {
  bitrev_shuffle256(a);
  ntt_ct_rev2std_v1(a, 256, psi_powers_ntt256_12289);
}

static void ntt256_v2(int32_t *a) {
  bitrev_shuffle256(a);
  ntt_ct_rev2std(a, 256, shoup_ntt256_12289);
}

static void ntt256_v3(int32_t *a) {
  ntt_ct_std2rev(a, 256, rev_shoup_ntt256_12289);
  bitrev_shuffle256(a);
}

static void ntt256_v4(int32_t *a) {
  bitrev_shuffle256(a);
  ntt_gs_rev2std(a, 256, rev_shoup_ntt256_12289);
}

static void ntt256_v5(int32_t *a) {
  bitrev_shuffle256(a);
  nttmul_gs_rev2std(a, 256, rev_shoup_ntt256_12289);
}

// Size 512
static void ntt512_v1(int32_t *a) {
  bitrev_shuffle512(a);
  ntt_ct_rev2std_v1(a, 512, psi_powers_ntt512_12289);
}

static void ntt512_v2(int32_t *a) {
  bitrev_shuffle512(a);
  ntt_ct_rev2std(a, 512, shoup_ntt512_12289);
}

static void ntt512_v3(int32_t *a) {
  ntt_ct_std2rev(a, 512, rev_shoup_ntt512_12289);
  bitrev_shuffle512(a);
}

static void ntt512_v4(int32_t *a) {
  bitrev_shuffle512(a);
  ntt_gs_rev2std(a, 512, rev_shoup_ntt512_12289);
}

static void ntt512_v5(int32_t *a) {
  bitrev_shuffle512(a);
  nttmul_gs_rev2std(a, 512, rev_shoup_ntt512_12289);
}

// Size 1024
static void ntt1024_v1(int32_t *a) {
  bitrev_shuffle1024(a);
  ntt_ct_rev2std_v1(a, 1024, psi_powers_ntt1024_12289);
}

static void ntt1024_v2(int32_t *a) {
  bitrev_shuffle1024(a);
  ntt_ct_rev2std(a, 1024, shoup_ntt1024_12289);
}

static void ntt1024_v3(int32_t *a) {
  ntt_ct_std2rev(a, 1024, rev_shoup_ntt1024_12289);
  bitrev_shuffle1024(a);
}

static void ntt1024_v4(int32_t *a) {
  bitrev_shuffle1024(a);
  ntt_gs_rev2std(a, 1024, rev_shoup_ntt1024_12289);
}

static void ntt1024_v5(int32_t *a) {
  bitrev_shuffle1024(a);
  nttmul_gs_rev2std(a, 1024, rev_shoup_ntt1024_12289);
}


/*
 * FOR CROSS-CHECKS
 */
// Size 16
static void mul_then_ntt16(int32_t *a) {
  mul_array16(a, 16, psi_powers_ntt16_12289);
  bitrev_shuffle(a, 16);
  ntt_ct_rev2std_v1(a, 16, psi_powers_ntt16_12289);
}

static void combined_mulntt16(int32_t *a) {
  bitrev_shuffle(a, 16);
  mulntt_ct_rev2std(a, 16, shoup_scaled_ntt16_12289);
}

static void mul_then_ntt16_rev(int32_t *a) {
  mul_array16(a, 16, psi_powers_ntt16_12289);
  ntt_ct_std2rev(a, 16, rev_shoup_ntt16_12289);
}

static void combined_mulntt16_rev(int32_t *a) {
  mulntt_ct_std2rev(a, 16, rev_shoup_scaled_ntt16_12289);
}

static void ntt_then_mul16(int32_t *a) {
  bitrev_shuffle(a, 16);
  ntt_ct_rev2std(a, 16, shoup_ntt16_12289);
  mul_array16(a, 16, psi_powers_ntt16_12289);
}

static void combined_nttmul16(int32_t *a) {
  bitrev_shuffle(a, 16);
  nttmul_gs_rev2std(a, 16, rev_shoup_scaled_ntt16_12289);
}

static void ntt_then_mul16_rev(int32_t *a) {
  ntt_ct_std2rev(a, 16, rev_shoup_ntt16_12289);
  bitrev_shuffle(a, 16);
  mul_array16(a, 16, psi_powers_ntt16_12289);
}

static void combined_nttmul16_rev(int32_t *a) {
  nttmul_gs_std2rev(a, 16, shoup_scaled_ntt16_12289);
  bitrev_shuffle(a, 16);
}

// Size 256
static void mul_then_ntt256(int32_t *a) {
  mul_array16(a, 256, psi_powers_ntt256_12289);
  bitrev_shuffle256(a);
  ntt_ct_rev2std_v1(a, 256, psi_powers_ntt256_12289);
}

static void combined_mulntt256(int32_t *a) {
  bitrev_shuffle256(a);
  mulntt_ct_rev2std(a, 256, shoup_scaled_ntt256_12289);
}

static void mul_then_ntt256_rev(int32_t *a) {
  mul_array16(a, 256, psi_powers_ntt256_12289);
  ntt_ct_std2rev(a, 256, rev_shoup_ntt256_12289);
}

static void combined_mulntt256_rev(int32_t *a) {
  mulntt_ct_std2rev(a, 256, rev_shoup_scaled_ntt256_12289);
}

static void ntt_then_mul256(int32_t *a) {
  bitrev_shuffle256(a);
  ntt_ct_rev2std(a, 256, shoup_ntt256_12289);
  mul_array16(a, 256, psi_powers_ntt256_12289);
}

static void combined_nttmul256(int32_t *a) {
  bitrev_shuffle256(a);
  nttmul_gs_rev2std(a, 256, rev_shoup_scaled_ntt256_12289);
}

static void ntt_then_mul256_rev(int32_t *a) {
  ntt_ct_std2rev(a, 256, rev_shoup_ntt256_12289);
  bitrev_shuffle(a, 256);
  mul_array16(a, 256, psi_powers_ntt256_12289);
}

static void combined_nttmul256_rev(int32_t *a) {
  nttmul_gs_std2rev(a, 256, shoup_scaled_ntt256_12289);
  bitrev_shuffle(a, 256);
}

// Size 512
static void mul_then_ntt512(int32_t *a) {
  mul_array16(a, 512, psi_powers_ntt512_12289);
  bitrev_shuffle512(a);
  ntt_ct_rev2std_v1(a, 512, psi_powers_ntt512_12289);
}

static void combined_mulntt512(int32_t *a) {
  bitrev_shuffle512(a);
  mulntt_ct_rev2std(a, 512, shoup_scaled_ntt512_12289);
}

static void mul_then_ntt512_rev(int32_t *a) {
  mul_array16(a, 512, psi_powers_ntt512_12289);
  ntt_ct_std2rev(a, 512, rev_shoup_ntt512_12289);
}

static void combined_mulntt512_rev(int32_t *a) {
  mulntt_ct_std2rev(a, 512, rev_shoup_scaled_ntt512_12289);
}

static void ntt_then_mul512(int32_t *a) {
  bitrev_shuffle512(a);
  ntt_ct_rev2std(a, 512, shoup_ntt512_12289);
  mul_array16(a, 512, psi_powers_ntt512_12289);
}

static void combined_nttmul512(int32_t *a) {
  bitrev_shuffle512(a);
  nttmul_gs_rev2std(a, 512, rev_shoup_scaled_ntt512_12289);
}

static void ntt_then_mul512_rev(int32_t *a) {
  ntt_ct_std2rev(a, 512, rev_shoup_ntt512_12289);
  bitrev_shuffle(a, 512);
  mul_array16(a, 512, psi_powers_ntt512_12289);
}

static void combined_nttmul512_rev(int32_t *a) {
  nttmul_gs_std2rev(a, 512, shoup_scaled_ntt512_12289);
  bitrev_shuffle(a, 512);
}

// Size 1024
static void mul_then_ntt1024(int32_t *a) {
  mul_array16(a, 1024, psi_powers_ntt1024_12289);
  bitrev_shuffle1024(a);
  ntt_ct_rev2std_v1(a, 1024, psi_powers_ntt1024_12289);
}

static void combined_mulntt1024(int32_t *a) {
  bitrev_shuffle1024(a);
  mulntt_ct_rev2std(a, 1024, shoup_scaled_ntt1024_12289);
}

static void mul_then_ntt1024_rev(int32_t *a) {
  mul_array16(a, 1024, psi_powers_ntt1024_12289);
  ntt_ct_std2rev(a, 1024, rev_shoup_ntt1024_12289);
}

static void combined_mulntt1024_rev(int32_t *a) {
  mulntt_ct_std2rev(a, 1024, rev_shoup_scaled_ntt1024_12289);
}

static void ntt_then_mul1024(int32_t *a) {
  bitrev_shuffle1024(a);
  ntt_ct_rev2std(a, 1024, shoup_ntt1024_12289);
  mul_array16(a, 1024, psi_powers_ntt1024_12289);
}

static void combined_nttmul1024(int32_t *a) {
  bitrev_shuffle1024(a);
  nttmul_gs_rev2std(a, 1024, rev_shoup_scaled_ntt1024_12289);
}

static void ntt_then_mul1024_rev(int32_t *a) {
  ntt_ct_std2rev(a, 1024, rev_shoup_ntt1024_12289);
  bitrev_shuffle(a, 1024);
  mul_array16(a, 1024, psi_powers_ntt1024_12289);
}

static void combined_nttmul1024_rev(int32_t *a) {
  nttmul_gs_std2rev(a, 1024, shoup_scaled_ntt1024_12289);
  bitrev_shuffle(a, 1024);
}


/*
 * Speed tests: skip the shuffle
 */
// Size 16
static void speed_ntt16_v1(int32_t *a) {
  ntt_ct_rev2std_v1(a, 16, psi_powers_ntt16_12289);
}

static void speed_ntt16_v2(int32_t *a) {
  ntt_ct_rev2std(a, 16, shoup_ntt16_12289);
}

static void speed_ntt16_v3(int32_t *a) {
  mulntt_ct_rev2std(a, 16, shoup_ntt16_12289);
}

static void speed_ntt16_v4(int32_t *a) {
  ntt_ct_std2rev(a, 16, rev_shoup_ntt16_12289);
}

static void speed_ntt16_v5(int32_t *a) {
  mulntt_ct_rev2std(a, 16, rev_shoup_ntt16_12289);
}

static void speed_ntt16_v6(int32_t *a) {
  ntt_gs_rev2std(a, 16, rev_shoup_ntt16_12289);
}

static void speed_ntt16_v7(int32_t *a) {
  nttmul_gs_rev2std(a, 16, rev_shoup_ntt16_12289);
}

// Size 256
static void speed_ntt256_v1(int32_t *a) {
  ntt_ct_rev2std_v1(a, 256, psi_powers_ntt256_12289);
}

static void speed_ntt256_v2(int32_t *a) {
  ntt_ct_rev2std(a, 256, shoup_ntt256_12289);
}

static void speed_ntt256_v3(int32_t *a) {
  ntt_ct_std2rev(a, 256, rev_shoup_ntt256_12289);
}

static void speed_ntt256_v4(int32_t *a) {
  ntt_gs_rev2std(a, 256, rev_shoup_ntt256_12289);
}

static void speed_ntt256_v5(int32_t *a) {
  nttmul_gs_rev2std(a, 256, rev_shoup_ntt256_12289);
}

// Size 512
static void speed_ntt512_v1(int32_t *a) {
  ntt_ct_rev2std_v1(a, 512, psi_powers_ntt512_12289);
}

static void speed_ntt512_v2(int32_t *a) {
  ntt_ct_rev2std(a, 512, shoup_ntt512_12289);
}

static void speed_ntt512_v3(int32_t *a) {
  ntt_ct_std2rev(a, 512, rev_shoup_ntt512_12289);
}

static void speed_ntt512_v4(int32_t *a) {
  ntt_gs_rev2std(a, 512, rev_shoup_ntt512_12289);
}

static void speed_ntt512_v5(int32_t *a) {
  nttmul_gs_rev2std(a, 512, rev_shoup_ntt512_12289);
}

// Size 1024
static void speed_ntt1024_v1(int32_t *a) {
  ntt_ct_rev2std_v1(a, 1024, psi_powers_ntt1024_12289);
}

static void speed_ntt1024_v2(int32_t *a) {
  ntt_ct_rev2std(a, 1024, shoup_ntt1024_12289);
}

static void speed_ntt1024_v3(int32_t *a) {
  ntt_ct_std2rev(a, 1024, rev_shoup_ntt1024_12289);
}

static void speed_ntt1024_v4(int32_t *a) {
  ntt_gs_rev2std(a, 1024, rev_shoup_ntt1024_12289);
}

static void speed_ntt1024_v5(int32_t *a) {
  nttmul_gs_rev2std(a, 1024, rev_shoup_ntt1024_12289);
}


int main(void) {
  // for n=16, omega = 6553
  test_simple_polys("version 1", 16, ntt16_v1, 6553);
  test_simple_polys("version 2", 16, ntt16_v2, 6553);
  test_simple_polys("version 3", 16, ntt16_v3, 6553);
  test_simple_polys("version 4", 16, ntt16_v4, 6553);
  test_simple_polys("version 5", 16, ntt16_v5, 6553);
  test_simple_polys("version 6", 16, ntt16_v6, 6553);
  test_simple_polys("version 7", 16, ntt16_v7, 6553);
  test_simple_polys("version 8", 16, ntt16_v8, 6553);
  test_simple_polys("version 9", 16, ntt16_v9, 6553);
  printf("\n");
  // for n=256, omega = 8595
  test_simple_polys("version 1", 256, ntt256_v1, 8595);
  test_simple_polys("version 2", 256, ntt256_v2, 8595);
  test_simple_polys("version 3", 256, ntt256_v3, 8595);
  test_simple_polys("version 4", 256, ntt256_v4, 8595);
  test_simple_polys("version 5", 256, ntt256_v5, 8595);
  printf("\n");
  // for n=512, omega = 10600
  test_simple_polys("version 1", 512, ntt512_v1, 10600);
  test_simple_polys("version 2", 512, ntt512_v2, 10600);
  test_simple_polys("version 3", 512, ntt512_v3, 10600);
  test_simple_polys("version 4", 512, ntt512_v4, 10600);
  test_simple_polys("version 5", 512, ntt512_v5, 10600);
  printf("\n");
  // for n=1024, omega = 8209
  test_simple_polys("version 1", 1024, ntt1024_v1, 8209);
  test_simple_polys("version 2", 1024, ntt1024_v2, 8209);
  test_simple_polys("version 3", 1024, ntt1024_v3, 8209);
  test_simple_polys("version 4", 1024, ntt1024_v4, 8209);
  test_simple_polys("version 5", 1024, ntt1024_v5, 8209);
  printf("\n");

  printf("\n");
  cross_check("mulntt", 16, combined_mulntt16, mul_then_ntt16);
  cross_check("mulntt", 256, combined_mulntt256, mul_then_ntt256);
  cross_check("mulntt", 512, combined_mulntt512, mul_then_ntt512);
  cross_check("mulntt", 1024, combined_mulntt1024, mul_then_ntt1024);

  printf("\n");
  cross_check("mulntt_rev", 16, combined_mulntt16_rev, mul_then_ntt16_rev);
  cross_check("mulntt_rev", 256, combined_mulntt256_rev, mul_then_ntt256_rev);
  cross_check("mulntt_rev", 512, combined_mulntt512_rev, mul_then_ntt512_rev);
  cross_check("mulntt_rev", 1024, combined_mulntt1024_rev, mul_then_ntt1024_rev);
  
  printf("\n");
  cross_check("nttmul", 16, combined_nttmul16, ntt_then_mul16);
  cross_check("nttmul", 256, combined_nttmul256, ntt_then_mul256);
  cross_check("nttmul", 512, combined_nttmul512, ntt_then_mul512);
  cross_check("nttmul", 1024, combined_nttmul1024, ntt_then_mul1024);

  printf("\n");
  cross_check("nttmul_rev", 16, combined_nttmul16_rev, ntt_then_mul16_rev);
  cross_check("nttmul_rev", 256, combined_nttmul256_rev, ntt_then_mul256_rev);
  cross_check("nttmul_rev", 512, combined_nttmul512_rev, ntt_then_mul512_rev);
  cross_check("nttmul_rev", 1024, combined_nttmul1024_rev, ntt_then_mul1024_rev);
  printf("\n");

  speed_test("ntt16_v1", 16, speed_ntt16_v1);
  speed_test("ntt16_v2", 16, speed_ntt16_v2);
  speed_test("ntt16_v3", 16, speed_ntt16_v3);
  speed_test("ntt16_v4", 16, speed_ntt16_v4);
  speed_test("ntt16_v5", 16, speed_ntt16_v5);
  speed_test("ntt16_v6", 16, speed_ntt16_v6);
  speed_test("ntt16_v7", 16, speed_ntt16_v7);
  printf("\n");
  speed_test("ntt256_v1", 256, speed_ntt256_v1);
  speed_test("ntt256_v2", 256, speed_ntt256_v2);
  speed_test("ntt256_v3", 256, speed_ntt256_v3);
  speed_test("ntt256_v4", 256, speed_ntt256_v4);
  speed_test("ntt256_v5", 256, speed_ntt256_v5);
  printf("\n");
  speed_test("ntt512_v1", 512, speed_ntt512_v1);
  speed_test("ntt512_v2", 512, speed_ntt512_v2);
  speed_test("ntt512_v3", 512, speed_ntt512_v3);
  speed_test("ntt512_v4", 512, speed_ntt512_v4);
  speed_test("ntt512_v5", 512, speed_ntt512_v5);
  printf("\n");
  speed_test("ntt1024_v1", 1024, speed_ntt1024_v1);
  speed_test("ntt1024_v2", 1024, speed_ntt1024_v2);
  speed_test("ntt1024_v3", 1024, speed_ntt1024_v3);
  speed_test("ntt1024_v4", 1024, speed_ntt1024_v4);
  speed_test("ntt1024_v5", 1024, speed_ntt1024_v5);

  printf("\n");
  speed_test("mul then ntt16", 16, mul_then_ntt16);
  speed_test("combined mul/ntt16", 16, combined_mulntt16);
  speed_test("mul then ntt16 std2rev", 16, mul_then_ntt16_rev);
  speed_test("combined mul/ntt16 std2rev", 16, combined_mulntt16_rev);
  speed_test("ntt then mul16", 16, ntt_then_mul16);
  speed_test("combined ntt/mul16", 16, combined_nttmul16);
  speed_test("ntt then mul16 std2rev", 16, ntt_then_mul16_rev);
  speed_test("combined ntt/mul16 std2rev", 16, combined_nttmul16_rev);
  printf("\n");
  speed_test("mul then ntt256", 256, mul_then_ntt256);
  speed_test("combined mul/ntt256", 256, combined_mulntt256);
  speed_test("mul then ntt256 std2rev", 256, mul_then_ntt256_rev);
  speed_test("combined mul/ntt256 std2rev", 256, combined_mulntt256_rev);
  speed_test("ntt then mul256", 256, ntt_then_mul256);
  speed_test("combined ntt/mul256", 256, combined_nttmul256);
  speed_test("ntt then mul256 std2rev", 256, ntt_then_mul256_rev);
  speed_test("combined ntt/mul256 std2rev", 256, combined_nttmul256_rev);
  printf("\n");
  speed_test("mul then ntt512", 512, mul_then_ntt512);
  speed_test("combined mul/ntt512", 512, combined_mulntt512);
  speed_test("mul then ntt512 std2rev", 512, mul_then_ntt512_rev);
  speed_test("combined mul/ntt512 std2rev", 512, combined_mulntt512_rev);
  speed_test("ntt then mul512", 512, ntt_then_mul512);
  speed_test("combined ntt/mul512", 512, combined_nttmul512);
  speed_test("ntt then mul512 std2rev", 512, ntt_then_mul512_rev);
  speed_test("combined ntt/mul512 std2rev", 512, combined_nttmul512_rev);
  printf("\n");
  speed_test("mul then ntt1024", 1024, mul_then_ntt1024);
  speed_test("combined mul/ntt1024", 1024, combined_mulntt1024);
  speed_test("mul then ntt1024 std2rev", 1024, mul_then_ntt1024_rev);
  speed_test("combined mul/ntt1024 std2rev", 1024, combined_mulntt1024_rev);
  speed_test("ntt then mul1024", 1024, ntt_then_mul1024);
  speed_test("combined ntt/mul1024", 1024, combined_nttmul1024);
  speed_test("ntt then mul1024 std2rev", 1024, ntt_then_mul1024_rev);
  speed_test("combined ntt/mul1024 std2rev", 1024, combined_nttmul1024_rev);
  printf("\n");

  return 0;
}
