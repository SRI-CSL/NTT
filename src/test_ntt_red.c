/*
 * Basic ntt tests
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ntt_red.h"
#include "ntt.h"
#include "red_bounds.h"
#include "test_bitrev_tables.h"
#include "test_ntt_red_tables.h"
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
static void print_array(FILE *f, int32_t *a, int32_t n) {
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



/*
 * BIT-REVERSE SHUFFLES
 */
static inline void bitrev_shuffle128(int32_t *a) {
  shuffle_with_table(a, bitrev128_pair, BITREV128_NPAIRS);
}

static inline void bitrev_shuffle256(int32_t *a) {
  shuffle_with_table(a, bitrev256_pair, BITREV256_NPAIRS);
}

static inline void bitrev_shuffle512(int32_t *a) {
  shuffle_with_table(a, bitrev512_pair, BITREV512_NPAIRS);
}

static inline void bitrev_shuffle1024(int32_t *a) {
  shuffle_with_table(a, bitrev1024_pair, BITREV1024_NPAIRS);
}

static inline void bitrev_shuffle2048(int32_t *a) {
  shuffle_with_table(a, bitrev2048_pair, BITREV2048_NPAIRS);
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


#if 0
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
      printf("correct result:\n");
      print_array(stdout, b, n);
      exit(1);
    }
  }
  printf("all tests passed\n");
}
#endif

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
static void ntt16_red_ct_rev2std(int32_t *a) {
  shift_array(a, 16);
  bitrev_shuffle(a, 16);
  ntt_red_ct_rev2std(a, 16, shoup_sred_ntt16_12289);
  normalize(a, 16);
}

static void ntt16_red_ct_std2rev(int32_t *a) {
  shift_array(a, 16);
  ntt_red_ct_std2rev(a, 16, rev_shoup_sred_ntt16_12289);
  normalize(a, 16);
  bitrev_shuffle(a, 16);
}

static void ntt16_red_gs_rev2std(int32_t *a) {
  shift_array(a, 16);
  bitrev_shuffle(a, 16);
  ntt_red_gs_rev2std(a, 16, rev_shoup_sred_ntt16_12289);
  normalize(a, 16);
}

static void ntt16_red_gs_std2rev(int32_t *a) {
  shift_array(a, 16);
  ntt_red_gs_std2rev(a, 16, shoup_sred_ntt16_12289);
  normalize(a, 16);
  bitrev_shuffle(a, 16);
}

static void mulntt16_red_ct_rev2std(int32_t *a) {
  shift_array(a, 16);
  bitrev_shuffle(a, 16);
  mulntt_red_ct_rev2std(a, 16, shoup_sred_ntt16_12289);
  normalize(a, 16);
}

static void mulntt16_red_ct_std2rev(int32_t *a) {
  shift_array(a, 16);
  mulntt_red_ct_std2rev(a, 16, rev_shoup_sred_ntt16_12289);
  normalize(a, 16);
  bitrev_shuffle(a, 16);
}

static void nttmul16_red_gs_rev2std(int32_t *a) {
  shift_array(a, 16);
  bitrev_shuffle(a, 16);
  nttmul_red_gs_rev2std(a, 16, rev_shoup_sred_ntt16_12289);
  normalize(a, 16);
}

static void nttmul16_red_gs_std2rev(int32_t *a) {
  shift_array(a, 16);
  nttmul_red_gs_std2rev(a, 16, shoup_sred_ntt16_12289);
  normalize(a, 16);
  bitrev_shuffle(a, 16);
}

// Size 128
static void ntt128_red_ct_rev2std(int32_t *a) {
  shift_array(a, 128);
  bitrev_shuffle128(a);
  ntt_red_ct_rev2std(a, 128, shoup_sred_ntt128_12289);
  normalize(a, 128);
}

static void ntt128_red_ct_std2rev(int32_t *a) {
  shift_array(a, 128);
  ntt_red_ct_std2rev(a, 128, rev_shoup_sred_ntt128_12289);
  normalize(a, 128);
  bitrev_shuffle128(a);
}

static void ntt128_red_gs_rev2std(int32_t *a) {
  shift_array(a, 128);
  bitrev_shuffle128(a);
  ntt_red_gs_rev2std(a, 128, rev_shoup_sred_ntt128_12289);
  normalize(a, 128);
}

static void ntt128_red_gs_std2rev(int32_t *a) {
  shift_array(a, 128);
  ntt_red_gs_std2rev(a, 128, shoup_sred_ntt128_12289);
  normalize(a, 128);
  bitrev_shuffle128(a);
}

static void mulntt128_red_ct_rev2std(int32_t *a) {
  shift_array(a, 128);
  bitrev_shuffle(a, 128);
  mulntt_red_ct_rev2std(a, 128, shoup_sred_ntt128_12289);
  normalize(a, 128);
}

static void mulntt128_red_ct_std2rev(int32_t *a) {
  shift_array(a, 128);
  mulntt_red_ct_std2rev(a, 128, rev_shoup_sred_ntt128_12289);
  normalize(a, 128);
  bitrev_shuffle(a, 128);
}

static void nttmul128_red_gs_rev2std(int32_t *a) {
  shift_array(a, 128);
  bitrev_shuffle(a, 128);
  nttmul_red_gs_rev2std(a, 128, rev_shoup_sred_ntt128_12289);
  normalize(a, 128);
}

static void nttmul128_red_gs_std2rev(int32_t *a) {
  shift_array(a, 128);
  nttmul_red_gs_std2rev(a, 128, shoup_sred_ntt128_12289);
  normalize(a, 128);
  bitrev_shuffle(a, 128);
}

// Size 256
static void ntt256_red_ct_rev2std(int32_t *a) {
  shift_array(a, 256);
  bitrev_shuffle256(a);
  ntt_red_ct_rev2std(a, 256, shoup_sred_ntt256_12289);
  normalize(a, 256);
}

static void ntt256_red_ct_std2rev(int32_t *a) {
  shift_array(a, 256);
  ntt_red_ct_std2rev(a, 256, rev_shoup_sred_ntt256_12289);
  normalize(a, 256);
  bitrev_shuffle256(a);
}

static void ntt256_red_gs_rev2std(int32_t *a) {
  shift_array(a, 256);
  bitrev_shuffle256(a);
  ntt_red_gs_rev2std(a, 256, rev_shoup_sred_ntt256_12289);
  normalize(a, 256);
}

static void ntt256_red_gs_std2rev(int32_t *a) {
  shift_array(a, 256);
  ntt_red_gs_std2rev(a, 256, shoup_sred_ntt256_12289);
  normalize(a, 256);
  bitrev_shuffle256(a);
}

static void mulntt256_red_ct_rev2std(int32_t *a) {
  shift_array(a, 256);
  bitrev_shuffle(a, 256);
  mulntt_red_ct_rev2std(a, 256, shoup_sred_ntt256_12289);
  normalize(a, 256);
}

static void mulntt256_red_ct_std2rev(int32_t *a) {
  shift_array(a, 256);
  mulntt_red_ct_std2rev(a, 256, rev_shoup_sred_ntt256_12289);
  normalize(a, 256);
  bitrev_shuffle(a, 256);
}

static void nttmul256_red_gs_rev2std(int32_t *a) {
  shift_array(a, 256);
  bitrev_shuffle(a, 256);
  nttmul_red_gs_rev2std(a, 256, rev_shoup_sred_ntt256_12289);
  normalize(a, 256);
}

static void nttmul256_red_gs_std2rev(int32_t *a) {
  shift_array(a, 256);
  nttmul_red_gs_std2rev(a, 256, shoup_sred_ntt256_12289);
  normalize(a, 256);
  bitrev_shuffle(a, 256);
}

// Size 512
static void ntt512_red_ct_rev2std(int32_t *a) {
  shift_array(a, 512);
  bitrev_shuffle512(a);
  ntt_red_ct_rev2std(a, 512, shoup_sred_ntt512_12289);
  normalize(a, 512);
}

static void ntt512_red_ct_std2rev(int32_t *a) {
  shift_array(a, 512);
  ntt_red_ct_std2rev(a, 512, rev_shoup_sred_ntt512_12289);
  normalize(a, 512);
  bitrev_shuffle512(a);
}

static void ntt512_red_gs_rev2std(int32_t *a) {
  shift_array(a, 512);
  bitrev_shuffle512(a);
  ntt_red_gs_rev2std(a, 512, rev_shoup_sred_ntt512_12289);
  normalize(a, 512);
}

static void ntt512_red_gs_std2rev(int32_t *a) {
  shift_array(a, 512);
  ntt_red_gs_std2rev(a, 512, shoup_sred_ntt512_12289);
  normalize(a, 512);
  bitrev_shuffle512(a);
}

static void mulntt512_red_ct_rev2std(int32_t *a) {
  shift_array(a, 512);
  bitrev_shuffle(a, 512);
  mulntt_red_ct_rev2std(a, 512, shoup_sred_ntt512_12289);
  normalize(a, 512);
}

static void mulntt512_red_ct_std2rev(int32_t *a) {
  shift_array(a, 512);
  mulntt_red_ct_std2rev(a, 512, rev_shoup_sred_ntt512_12289);
  normalize(a, 512);
  bitrev_shuffle(a, 512);
}

static void nttmul512_red_gs_rev2std(int32_t *a) {
  shift_array(a, 512);
  bitrev_shuffle(a, 512);
  nttmul_red_gs_rev2std(a, 512, rev_shoup_sred_ntt512_12289);
  normalize(a, 512);
}

static void nttmul512_red_gs_std2rev(int32_t *a) {
  shift_array(a, 512);
  nttmul_red_gs_std2rev(a, 512, shoup_sred_ntt512_12289);
  normalize(a, 512);
  bitrev_shuffle(a, 512);
}

// Size 1024
static void ntt1024_red_ct_rev2std(int32_t *a) {
  shift_array(a, 1024);
  bitrev_shuffle1024(a);
  ntt_red_ct_rev2std(a, 1024, shoup_sred_ntt1024_12289);
  normalize(a, 1024);
}

static void ntt1024_red_ct_std2rev(int32_t *a) {
  shift_array(a, 1024);
  ntt_red_ct_std2rev(a, 1024, rev_shoup_sred_ntt1024_12289);
  normalize(a, 1024);
  bitrev_shuffle1024(a);
}

static void ntt1024_red_gs_rev2std(int32_t *a) {
  shift_array(a, 1024);
  bitrev_shuffle1024(a);
  ntt_red_gs_rev2std(a, 1024, rev_shoup_sred_ntt1024_12289);
  normalize(a, 1024);
}

static void ntt1024_red_gs_std2rev(int32_t *a) {
  shift_array(a, 1024);
  ntt_red_gs_std2rev(a, 1024, shoup_sred_ntt1024_12289);
  normalize(a, 1024);
  bitrev_shuffle1024(a);
}

static void mulntt1024_red_ct_rev2std(int32_t *a) {
  shift_array(a, 1024);
  bitrev_shuffle(a, 1024);
  mulntt_red_ct_rev2std(a, 1024, shoup_sred_ntt1024_12289);
  normalize(a, 1024);
}

static void mulntt1024_red_ct_std2rev(int32_t *a) {
  shift_array(a, 1024);
  mulntt_red_ct_std2rev(a, 1024, rev_shoup_sred_ntt1024_12289);
  normalize(a, 1024);
  bitrev_shuffle(a, 1024);
}

static void nttmul1024_red_gs_rev2std(int32_t *a) {
  shift_array(a, 1024);
  bitrev_shuffle(a, 1024);
  nttmul_red_gs_rev2std(a, 1024, rev_shoup_sred_ntt1024_12289);
  normalize(a, 1024);
}

static void nttmul1024_red_gs_std2rev(int32_t *a) {
  shift_array(a, 1024);
  nttmul_red_gs_std2rev(a, 1024, shoup_sred_ntt1024_12289);
  normalize(a, 1024);
  bitrev_shuffle(a, 1024);
}

// Size 2048
static void ntt2048_red_ct_rev2std(int32_t *a) {
  shift_array(a, 2048);
  bitrev_shuffle2048(a);
  ntt_red_ct_rev2std(a, 2048, shoup_sred_ntt2048_12289);
  normalize(a, 2048);
}

static void ntt2048_red_ct_std2rev(int32_t *a) {
  shift_array(a, 2048);
  ntt_red_ct_std2rev(a, 2048, rev_shoup_sred_ntt2048_12289);
  normalize(a, 2048);
  bitrev_shuffle2048(a);
}

static void ntt2048_red_gs_rev2std(int32_t *a) {
  shift_array(a, 2048);
  bitrev_shuffle2048(a);
  ntt_red_gs_rev2std(a, 2048, rev_shoup_sred_ntt2048_12289);
  normalize(a, 2048);
}

static void ntt2048_red_gs_std2rev(int32_t *a) {
  shift_array(a, 2048);
  ntt_red_gs_std2rev(a, 2048, shoup_sred_ntt2048_12289);
  normalize(a, 2048);
  bitrev_shuffle2048(a);
}

static void mulntt2048_red_ct_rev2std(int32_t *a) {
  shift_array(a, 2048);
  bitrev_shuffle(a, 2048);
  mulntt_red_ct_rev2std(a, 2048, shoup_sred_ntt2048_12289);
  normalize(a, 2048);
}

static void mulntt2048_red_ct_std2rev(int32_t *a) {
  shift_array(a, 2048);
  mulntt_red_ct_std2rev(a, 2048, rev_shoup_sred_ntt2048_12289);
  normalize(a, 2048);
  bitrev_shuffle(a, 2048);
}

static void nttmul2048_red_gs_rev2std(int32_t *a) {
  shift_array(a, 2048);
  bitrev_shuffle(a, 2048);
  nttmul_red_gs_rev2std(a, 2048, rev_shoup_sred_ntt2048_12289);
  normalize(a, 2048);
}

static void nttmul2048_red_gs_std2rev(int32_t *a) {
  shift_array(a, 2048);
  nttmul_red_gs_std2rev(a, 2048, shoup_sred_ntt2048_12289);
  normalize(a, 2048);
  bitrev_shuffle(a, 2048);
}

/*
 * SPEED TEST: skip normalization and shuffle
 */
// Size 16
static void speed_ntt16_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 16, shoup_sred_ntt16_12289);
}

static void speed_ntt16_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 16, rev_shoup_sred_ntt16_12289);
}

static void speed_ntt16_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 16, rev_shoup_sred_ntt16_12289);
}

static void speed_ntt16_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 16, shoup_sred_ntt16_12289);
}

// Size 128
static void speed_ntt128_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 128, shoup_sred_ntt128_12289);
}

static void speed_ntt128_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 128, rev_shoup_sred_ntt128_12289);
}

static void speed_ntt128_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 128, rev_shoup_sred_ntt128_12289);
}

static void speed_ntt128_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 128, shoup_sred_ntt128_12289);
}

// Size 256
static void speed_ntt256_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 256, shoup_sred_ntt256_12289);
}

static void speed_ntt256_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 256, rev_shoup_sred_ntt256_12289);
}

static void speed_ntt256_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 256, rev_shoup_sred_ntt256_12289);
}

static void speed_ntt256_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 256, shoup_sred_ntt256_12289);
}

// Size 512
static void speed_ntt512_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 512, shoup_sred_ntt512_12289);
}

static void speed_ntt512_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 512, rev_shoup_sred_ntt512_12289);
}

static void speed_ntt512_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 512, rev_shoup_sred_ntt512_12289);
}

static void speed_ntt512_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 512, shoup_sred_ntt512_12289);
}

// Size 1024
static void speed_ntt1024_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 1024, shoup_sred_ntt1024_12289);
}

static void speed_ntt1024_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 1024, rev_shoup_sred_ntt1024_12289);
}

static void speed_ntt1024_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 1024, rev_shoup_sred_ntt1024_12289);
}

static void speed_ntt1024_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 1024, shoup_sred_ntt1024_12289);
}

static void speed_mulntt1024_red_ct_rev2std(int32_t *a) {
  mulntt_red_ct_rev2std(a, 1024, shoup_sred_ntt1024_12289);
}

static void speed_mulntt1024_red_ct_std2rev(int32_t *a) {
  mulntt_red_ct_std2rev(a, 1024, rev_shoup_sred_ntt1024_12289);
}

static void speed_nttmul1024_red_gs_rev2std(int32_t *a) {
  nttmul_red_gs_rev2std(a, 1024, rev_shoup_sred_ntt1024_12289);
}

static void speed_nttmul1024_red_gs_std2rev(int32_t *a) {
  nttmul_red_gs_std2rev(a, 1024, shoup_sred_ntt1024_12289);
}

// Size 2048
static void speed_ntt2048_red_ct_rev2std(int32_t *a) {
  ntt_red_ct_rev2std(a, 2048, shoup_sred_ntt2048_12289);
}

static void speed_ntt2048_red_ct_std2rev(int32_t *a) {
  ntt_red_ct_std2rev(a, 2048, rev_shoup_sred_ntt2048_12289);
}

static void speed_ntt2048_red_gs_rev2std(int32_t *a) {
  ntt_red_gs_rev2std(a, 2048, rev_shoup_sred_ntt2048_12289);
}

static void speed_ntt2048_red_gs_std2rev(int32_t *a) {
  ntt_red_gs_std2rev(a, 2048, shoup_sred_ntt2048_12289);
 }

static void speed_mulntt2048_red_ct_rev2std(int32_t *a) {
  mulntt_red_ct_rev2std(a, 2048, shoup_sred_ntt2048_12289);
}

static void speed_mulntt2048_red_ct_std2rev(int32_t *a) {
  mulntt_red_ct_std2rev(a, 2048, rev_shoup_sred_ntt2048_12289);
}

static void speed_nttmul2048_red_gs_rev2std(int32_t *a) {
  nttmul_red_gs_rev2std(a, 2048, rev_shoup_sred_ntt2048_12289);
}

static void speed_nttmul2048_red_gs_std2rev(int32_t *a) {
  nttmul_red_gs_std2rev(a, 2048, shoup_sred_ntt2048_12289);
}


// Speed of normalization
static void speed_normalize_512(int32_t *a) {
  normalize(a, 512);
}

static void speed_normalize_1024(int32_t *a) {
  normalize(a, 1024);
}

static void speed_normalize_2048(int32_t *a) {
  normalize(a, 2048);
}



int main(void) {
  // for n=16, omega = 6553
  test_simple_polys("ntt16_red_ct_rev2std", 16, ntt16_red_ct_rev2std, 6553);
  test_simple_polys("ntt16_red_ct_std2rev", 16, ntt16_red_ct_std2rev, 6553);
  test_simple_polys("ntt16_red_gs_rev2std", 16, ntt16_red_gs_rev2std, 6553);
  test_simple_polys("ntt16_red_gs_std2rev", 16, ntt16_red_gs_std2rev, 6553);
  test_simple_polys("mulntt16_red_ct_rev2std", 16, mulntt16_red_ct_rev2std, 6553);
  test_simple_polys("mulntt16_red_ct_std2rev", 16, mulntt16_red_ct_std2rev, 6553);
  test_simple_polys("nttmul16_red_gs_rev2std", 16, nttmul16_red_gs_rev2std, 6553);
  test_simple_polys("nttmul16_red_gs_std2rev", 16, nttmul16_red_gs_std2rev, 6553);
  printf("\n");

  // for n=128, omega = 12208
  test_simple_polys("ntt128_red_ct_rev2std", 128, ntt128_red_ct_rev2std, 12208);
  test_simple_polys("ntt128_red_ct_std2rev", 128, ntt128_red_ct_std2rev, 12208);
  test_simple_polys("ntt128_red_gs_rev2std", 128, ntt128_red_gs_rev2std, 12208);
  test_simple_polys("ntt128_red_gs_std2rev", 128, ntt128_red_gs_std2rev, 12208);
  test_simple_polys("mulntt128_red_ct_rev2std", 128, mulntt128_red_ct_rev2std, 12208);
  test_simple_polys("mulntt128_red_ct_std2rev", 128, mulntt128_red_ct_std2rev, 12208);
  test_simple_polys("nttmul128_red_gs_rev2std", 128, nttmul128_red_gs_rev2std, 12208);
  test_simple_polys("nttmul128_red_gs_std2rev", 128, nttmul128_red_gs_std2rev, 12208);
  printf("\n");

  // for n=256, omega = 8595
  test_simple_polys("ntt256_red_ct_rev2std", 256, ntt256_red_ct_rev2std, 8595);
  test_simple_polys("ntt256_red_ct_std2rev", 256, ntt256_red_ct_std2rev, 8595);
  test_simple_polys("ntt256_red_gs_rev2std", 256, ntt256_red_gs_rev2std, 8595);
  test_simple_polys("ntt256_red_gs_std2rev", 256, ntt256_red_gs_std2rev, 8595);
  test_simple_polys("mulntt256_red_ct_rev2std", 256, mulntt256_red_ct_rev2std, 8595);
  test_simple_polys("mulntt256_red_ct_std2rev", 256, mulntt256_red_ct_std2rev, 8595);
  test_simple_polys("nttmul256_red_gs_rev2std", 256, nttmul256_red_gs_rev2std, 8595);
  test_simple_polys("nttmul256_red_gs_std2rev", 256, nttmul256_red_gs_std2rev, 8595);
  printf("\n");

  // for n=512, omega = 10600
  test_simple_polys("ntt512_red_ct_rev2std", 512, ntt512_red_ct_rev2std, 10600);
  test_simple_polys("ntt512_red_ct_std2rev", 512, ntt512_red_ct_std2rev, 10600);
  test_simple_polys("ntt512_red_gs_rev2std", 512, ntt512_red_gs_rev2std, 10600);
  test_simple_polys("ntt512_red_gs_std2rev", 512, ntt512_red_gs_std2rev, 10600);
  test_simple_polys("mulntt512_red_ct_rev2std", 512, mulntt512_red_ct_rev2std, 10600);
  test_simple_polys("mulntt512_red_ct_std2rev", 512, mulntt512_red_ct_std2rev, 10600);
  test_simple_polys("nttmul512_red_gs_rev2std", 512, nttmul512_red_gs_rev2std, 10600);
  test_simple_polys("nttmul512_red_gs_std2rev", 512, nttmul512_red_gs_std2rev, 10600);
  printf("\n");

  // for n=1024, omega = 8209
  test_simple_polys("ntt1024_red_ct_rev2std", 1024, ntt1024_red_ct_rev2std, 8209);
  test_simple_polys("ntt1024_red_ct_std2rev", 1024, ntt1024_red_ct_std2rev, 8209);
  test_simple_polys("ntt1024_red_gs_rev2std", 1024, ntt1024_red_gs_rev2std, 8209);
  test_simple_polys("ntt1024_red_gs_std2rev", 1024, ntt1024_red_gs_std2rev, 8209);
  test_simple_polys("mulntt1024_red_ct_rev2std", 1024, mulntt1024_red_ct_rev2std, 8209);
  test_simple_polys("mulntt1024_red_ct_std2rev", 1024, mulntt1024_red_ct_std2rev, 8209);
  test_simple_polys("nttmul1024_red_gs_rev2std", 1024, nttmul1024_red_gs_rev2std, 8209);
  test_simple_polys("nttmul1024_red_gs_std2rev", 1024, nttmul1024_red_gs_std2rev, 8209);
  printf("\n");

  // for n=2048, omega = 12269
  test_simple_polys("ntt2048_red_ct_rev2std", 2048, ntt2048_red_ct_rev2std, 12269);
  test_simple_polys("ntt2048_red_ct_std2rev", 2048, ntt2048_red_ct_std2rev, 12269);
  test_simple_polys("ntt2048_red_gs_rev2std", 2048, ntt2048_red_gs_rev2std, 12269);
  test_simple_polys("ntt2048_red_gs_std2rev", 2048, ntt2048_red_gs_std2rev, 12269);
  test_simple_polys("mulntt2048_red_ct_rev2std", 2048, mulntt2048_red_ct_rev2std, 12269);
  test_simple_polys("mulntt2048_red_ct_std2rev", 2048, mulntt2048_red_ct_std2rev, 12269);
  test_simple_polys("nttmul2048_red_gs_rev2std", 2048, nttmul2048_red_gs_rev2std, 12269);
  test_simple_polys("nttmul2048_red_gs_std2rev", 2048, nttmul2048_red_gs_std2rev, 12269);
  printf("\n");

  // speed measurements
  speed_test("ntt16_red_ct_rev2std", 16, speed_ntt16_red_ct_rev2std);
  speed_test("ntt16_red_ct_std2rev", 16, speed_ntt16_red_ct_std2rev);
  speed_test("ntt16_red_gs_rev2std", 16, speed_ntt16_red_gs_rev2std);
  speed_test("ntt16_red_gs_std2rev", 16, speed_ntt16_red_gs_std2rev);
  printf("\n");

  speed_test("ntt128_red_ct_rev2std", 128, speed_ntt128_red_ct_rev2std);
  speed_test("ntt128_red_ct_std2rev", 128, speed_ntt128_red_ct_std2rev);
  speed_test("ntt128_red_gs_rev2std", 128, speed_ntt128_red_gs_rev2std);
  speed_test("ntt128_red_gs_std2rev", 128, speed_ntt128_red_gs_std2rev);
  printf("\n");

  speed_test("ntt256_red_ct_rev2std", 256, speed_ntt256_red_ct_rev2std);
  speed_test("ntt256_red_ct_std2rev", 256, speed_ntt256_red_ct_std2rev);
  speed_test("ntt256_red_gs_rev2std", 256, speed_ntt256_red_gs_rev2std);
  speed_test("ntt256_red_gs_std2rev", 256, speed_ntt256_red_gs_std2rev);
  printf("\n");

  speed_test("ntt512_red_ct_rev2std", 512, speed_ntt512_red_ct_rev2std);
  speed_test("ntt512_red_ct_std2rev", 512, speed_ntt512_red_ct_std2rev);
  speed_test("ntt512_red_gs_rev2std", 512, speed_ntt512_red_gs_rev2std);
  speed_test("ntt512_red_gs_std2rev", 512, speed_ntt512_red_gs_std2rev);
  printf("\n");

  speed_test("ntt1024_red_ct_rev2std", 1024, speed_ntt1024_red_ct_rev2std);
  speed_test("ntt1024_red_ct_std2rev", 1024, speed_ntt1024_red_ct_std2rev);
  speed_test("ntt1024_red_gs_rev2std", 1024, speed_ntt1024_red_gs_rev2std);
  speed_test("ntt1024_red_gs_std2rev", 1024, speed_ntt1024_red_gs_std2rev);
  printf("\n");

  speed_test("mulntt1024_red_ct_rev2std", 1024, speed_mulntt1024_red_ct_rev2std);
  speed_test("mulntt1024_red_ct_std2rev", 1024, speed_mulntt1024_red_ct_std2rev);
  speed_test("nttmul1024_red_gs_rev2std", 1024, speed_nttmul1024_red_gs_rev2std);
  speed_test("nttmul1024_red_gs_std2rev", 1024, speed_nttmul1024_red_gs_std2rev);
  printf("\n");

  speed_test("ntt2048_red_ct_rev2std", 2048, speed_ntt2048_red_ct_rev2std);
  speed_test("ntt2048_red_ct_std2rev", 2048, speed_ntt2048_red_ct_std2rev);
  speed_test("ntt2048_red_gs_rev2std", 2048, speed_ntt2048_red_gs_rev2std);
  speed_test("ntt2048_red_gs_std2rev", 2048, speed_ntt2048_red_gs_std2rev);
  printf("\n");

  speed_test("mulntt2048_red_ct_rev2std", 2048, speed_mulntt2048_red_ct_rev2std);
  speed_test("mulntt2048_red_ct_std2rev", 2048, speed_mulntt2048_red_ct_std2rev);
  speed_test("nttmul2048_red_gs_rev2std", 2048, speed_nttmul2048_red_gs_rev2std);
  speed_test("nttmul2048_red_gs_std2rev", 2048, speed_nttmul2048_red_gs_std2rev);
  printf("\n");

  speed_test("normalize_512", 512, speed_normalize_512);
  speed_test("normalize_1024", 1024, speed_normalize_1024);
  speed_test("normalize_2048", 2048, speed_normalize_2048);
  printf("\n");

  return 0;
}
