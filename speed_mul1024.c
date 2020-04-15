#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

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

static void print_results(const char *s, uint64_t c) {
  uint32_t i;

  for(i=0 ;i<NTESTS-1; i++) {
    t[i] = t[i+1] - t[i];
  }
  t[i] = c - t[i];

  printf("%s\n", s);
  printf("median: %"PRIu64"\n", median_time());
  printf("average: %"PRIu64"\n", average_time());
  printf("\n");
}

static void test_mul(void) {
  int32_t a[1024], b[1024], c[1024];
  uint32_t i;

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }
  
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    ntt1024_product1(c, a, b);
  }
  print_results("ntt1024_product1 ", cpucycles());

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }
  
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    ntt1024_product2(c, a, b);
  }
  print_results("ntt1024_product2 ", cpucycles());

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }
  
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    ntt1024_product3(c, a, b);
  }
  print_results("ntt1024_product3 ", cpucycles());

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }
  
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    ntt1024_product4(c, a, b);
  }
  print_results("ntt1024_product4 ", cpucycles());

  for (i=0; i<1024; i++) {
    a[i] = i;
    b[i] = i;
  }
  
  for (i=0; i<NTESTS; i++) {
    t[i] = cpucycles();
    ntt1024_product5(c, a, b);
  }
  print_results("ntt1024_product5 ", cpucycles());
}

int main(void){
  printf("Testing ntt1024 product functions\n\n");
  test_mul();
  return 0;
}
