#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "red_bounds.h"
#include "test_ntt_red_tables.h"

static int64_t red(int64_t x) {
  return (3 * (x & 4095)) - (x >> 12);
}


// maximum of |red(x)| for |x| <= b
static int64_t max_abs_red(int64_t b) {
  int64_t min, max, m;

  min = min_red(-b, b, &m);
  max = max_red(-b, b, &m);
  // min <= red(x) <= max
  if (max < 0) return - min;
  if (min > 0) return max;
  min = -min;
  return (max > min) ? max : min;
}

static void ct_iteration(int64_t b, int64_t low, int64_t high) {
  uint32_t i;
  int64_t nb;

  printf("CT bounds\n");
  for (i=0; i<20; i++) {
    printf("  B%"PRIu32" = %"PRId64"\n", i, b);
    nb = ct_bound(b, low, high);
    if (nb > INT32_MAX) {
      nb = max_abs_red(b);
      printf("  Reduction to %"PRId64"\n", nb);
      nb = ct_bound(nb, low, high);
    }
    b = nb;
  }
  printf("\n");
}


static void gs_iteration(int64_t b, int64_t low, int64_t high) {
  uint32_t i;
  int64_t nb;

  printf("GS bounds\n");
  for (i=0; i<20; i++) {
    printf("  B%"PRIu32" = %"PRId64"\n", i, b);
    nb = gs_bound(b, low, high);
    if (nb > INT32_MAX) {
      nb = max_abs_red(b);
      printf("  Reduction to %"PRId64"\n", nb);
      nb = gs_bound(nb, low, high);
    }
    b = nb;
  }
  printf("\n");
}


/*
 * TESTS
 */

 /*
  * Test functions: if double_check is true, we do a slow check of
  * the results.
  */
 static void test_min_max_red(bool double_check) {
   int64_t min, max, min_x, max_x, b, x, r;
   uint32_t l;

   for (b = 128, l = 7; b <= (int64_t) 1 << 32; b *= 2, l++) {
     min = min_red(-b, b, &min_x);
     max = max_red(-b, b, &max_x);

     printf("bound on |x|: b = 2^%"PRIu32" = %"PRId64"\n", l, b);
     printf("  min_red = %"PRId64" (for x=%"PRId64")\n", min, min_x);
     printf("  max_red = %"PRId64" (for x=%"PRId64")\n", max, max_x);

     if (double_check) {
       for (x=-b; x<=b ; x++) {
	 r = red(x);
	 //    printf("red(%"PRId64") = %"PRId64"\n", x, r);
	 if (r < min || r > max) {
	   printf("BUG: red(%"PRId64") = %"PRId64"\n", x, r);
	   exit(EXIT_FAILURE);
	 }
       }
       printf("checked\n");
     }

     printf("\n");
   }
 }

 static void test_min_max_red_mul(bool double_check) {
   int64_t min, max, min_x, max_x, b, x, r, w;
   uint32_t l;

   for (w=1; w <= 12288; w++) {
     for (b = 128, l = 7; b <= (int64_t) 1 << 23; b *= 2, l++) {
       min = min_red_mul(-b, b, w, &min_x);
       max = max_red_mul(-b, b, w, &max_x);

       printf("bound on |x|: b = 2^%"PRIu32" = %"PRId64"\n", l, b);
       printf("coeff w = %"PRId64"\n", w);
       printf("  min red(w * x) = %"PRId64" (for x=%"PRId64")\n", min, min_x);
       printf("  max_red(w * x) = %"PRId64" (for x=%"PRId64")\n", max, max_x);

       if (double_check) {
	 for (x=-b; x<=b ; x ++) {
	   r = red(x * w);
	   //    printf("red(%"PRId64") = %"PRId64"\n", x, r);
	   if (r < min || r > max) {
	     printf("BUG: red(%"PRId64") = %"PRId64"\n", x, r);
	     exit(EXIT_FAILURE);
	   }
	 }
	 printf("checked\n");
       }

       printf("\n");
     }
   }

   for (w=-1; w >= -12288; w --) {
     for (b = 128, l = 7; b <= (int64_t) 1 << 23; b *= 2, l++) {
       min = min_red_mul(-b, b, w, &min_x);
       max = max_red_mul(-b, b, w, &max_x);

       printf("bound on |x|: b = 2^%"PRIu32" = %"PRId64"\n", l, b);
       printf("coeff w = %"PRId64"\n", w);
       printf("  min red(w * x) = %"PRId64" (for x=%"PRId64")\n", min, min_x);
       printf("  max red(w * x) = %"PRId64" (for x=%"PRId64")\n", max, max_x);

       if (double_check) {
	 for (x=-b; x<=b ; x ++) {
	   r = red(x * w);
	   //    printf("red(%"PRId64") = %"PRId64"\n", x, r);
	   if (r < min || r > max) {
	     printf("BUG: red(%"PRId64" * %"PRId64") = %"PRId64"\n", w, x, r);
	     exit(EXIT_FAILURE);
	   }
	 }
	 printf("checked\n");
       }

       printf("\n");
     }
   }
 }

 static void test_min_max_red_mul_interval(bool double_check) {
   int64_t min, max, min_x, max_x, min_w, max_w, a, b, x, r, w;
   uint32_t l;

   for (w=12288; w >= 12288/8; w = w/2) {
     for (b=128, l=7; b <= (int64_t) 1 << 32; b *= 2, l++) {
       printf("bound on |x|: b = 2^%"PRIu32" = %"PRId64"\n", l, b);
       printf("bound on |w|: c = %"PRId64"\n", w);
       fflush(stdout);

       min = min_red_mul_interval(-b, b, -w, w, &min_x, &min_w);
       max = max_red_mul_interval(-b, b, -w, w, &max_x, &max_w);

       printf("  min red(w * x) = %"PRId64" (for x=%"PRId64", w=%"PRId64")\n", min, min_x, min_w);
       printf("  max red(w * x) = %"PRId64" (for x=%"PRId64", w=%"PRId64")\n", max, max_x, max_w);

       if (double_check) {
	 a = (w<1000) ? -a : a - 1000;
	 while (a <= w) {
	   x = (b<30000) ? -b : b-30000;
	   while (x <= b) {
	     r = red(x * a);
	     //	      printf("red(%"PRId64" * %"PRId64") = %"PRId64"\n", a, x, r);
	     if (r < min || r > max) {
	       printf("BUG: red(%"PRId64" * %"PRId64") = %"PRId64"\n", a, x, r);
	       exit(EXIT_FAILURE);
	     }
	     r = red(-x * a);
	     //	      printf("red(%"PRId64" * %"PRId64") = %"PRId64"\n", a, -x, r);
	     if (r < min || r > max) {
	       printf("BUG: red(%"PRId64" * %"PRId64") = %"PRId64"\n", a, -x, r);
	       exit(EXIT_FAILURE);
	     }
	     x ++;
	   }
	   a ++;
	 }
	 printf("checked\n");	  
       }
       printf("\n");
     }
   }
 }


/*
 * Bounds after NTT computations
 */
static void show_ct_bounds(const char *name, uint32_t n, const int16_t *p) {
  int64_t b0, b;
  int64_t bound[12];

  assert(n <= 2048);

  printf("Bounds for function %s\n\n", name);
  b0 = 6144;
  printf("ct: bound on input = %"PRId64"\n", b0);
  b = ntt_ct_bounds(b0, n, p, bound);
  printf("ct: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");

  printf("gs: bound on input = %"PRId64"\n", b0);
  b = ntt_gs_bounds(b0, n, p, bound);
  printf("gs: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("\n");

  b0 = 12288;
  printf("ct: bound on input = %"PRId64"\n", b0);
  b = ntt_ct_bounds(b0, n, p, bound);
  printf("ct: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");

  printf("gs: bound on input = %"PRId64"\n", b0);
  b = ntt_gs_bounds(b0, n, p, bound);
  printf("gs: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("\n");

  b0 = 21499;
  printf("ct: bound on input = %"PRId64"\n", b0);
  b = ntt_ct_bounds(b0, n, p, bound);
  printf("ct: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");

  printf("gs: bound on input = %"PRId64"\n", b0);
  b = ntt_gs_bounds(b0, n, p, bound);
  printf("gs: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("\n");

  b0 = 30709;
  printf("ct: bound on input = %"PRId64"\n", b0);
  b = ntt_ct_bounds(b0, n, p, bound);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("ct: bound on output = %"PRId64"\n", b);

  printf("gs: bound on input = %"PRId64"\n", b0);
  b = ntt_gs_bounds(b0, n, p, bound);
  printf("gs: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("\n");

  b0 = 36864;
  printf("ct: bound on input = %"PRId64"\n", b0);
  b = ntt_ct_bounds(b0, n, p, bound);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("ct: bound on output = %"PRId64"\n", b);

  printf("gs: bound on input = %"PRId64"\n", b0);
  b = ntt_gs_bounds(b0, n, p, bound);
  printf("gs: bound on output = %"PRId64"\n", b);
  if (b >= INT32_MAX) printf("--> overflow\n");
  printf("\n");
}


int main(void) {
  int64_t min, max, min_x, max_x, min_y, max_y, b, nb, a, na;

  if (false) {
    test_min_max_red(true);
    test_min_max_red_mul(true);
    test_min_max_red_mul_interval(true);
  }

  printf("\nBounds after repeated reductions\n\n");
  na = INT32_MIN;
  nb = INT32_MAX;
  do {
    a = na;
    b = nb;
    printf("bounds: a <= x <= b,  a = %"PRId64", b = %"PRId64"\n", a, b);
    min = min_red(a, b, &min_x);
    max = max_red(a, b, &max_x);
    printf("  min red(x) = %"PRId64" (for x=%"PRId64")\n", min, min_x);
    printf("  max red(x) = %"PRId64" (for x=%"PRId64")\n", max, max_x);
    printf("\n");

    na = min;
    nb = max;

  } while (nb < b || na > a);


  printf("\nBounds on absolute value after repeated reductions\n\n");
  na = INT32_MIN;
  nb = INT32_MAX;
  do {
    a = na;
    b = nb;
    printf("bounds: a <= x <= b,  a = %"PRId64", b = %"PRId64"\n", a, b);
    min = min_red(a, b, &min_x);
    max = max_red(a, b, &max_x);
    printf("  min red(x) = %"PRId64" (for x=%"PRId64")\n", min, min_x);
    printf("  max red(x) = %"PRId64" (for x=%"PRId64")\n", max, max_x);
    printf("\n");

    if (min < 0) min = -min;
    if (max < 0) max = -max;
    if (min < max) {
      na = -max;
      nb = max;
    } else {
      na = -min;
      nb = min;
    }

  } while (nb < b || na > a);


  printf("\nBounds on reduced product\n\n");
  a = min_red(INT32_MIN, INT32_MAX, &min_x);
  b = max_red(INT32_MIN, INT32_MAX, &max_x);
  min = min_red_mul_interval(a, b, a, b, &min_x, &min_y);
  max = max_red_mul_interval(a, b, a, b, &max_x, &max_y);
  printf("After one reduction:\n");
  printf("  %"PRId64" <= x <= %"PRId64", %"PRId64" <= y <= %"PRId64"\n", a, b, a, b);
  printf("  %"PRId64" <= red(x * y) <= %"PRId64"\n\n", min, max);

  na = min_red(a, b, &min_x);
  nb = max_red(a, b, &max_x);;
  min = min_red_mul_interval(na, nb, na, nb, &min_x, &min_y);
  max = max_red_mul_interval(na, nb, na, nb, &max_x, &max_y);
  printf("After two reductions:\n");
  printf("  %"PRId64" <= x <= %"PRId64", %"PRId64" <= y <= %"PRId64"\n", na, nb, na, nb);
  printf("  %"PRId64" <= red(x * y) <= %"PRId64"\n\n", min, max);

  printf("\nBase CT iterations\n\n");
  printf("Option 1: B0 = 12288, 0 <= w <= 12288\n\n");
  ct_iteration(12288, 0, 12288);
  printf("\n");

  printf("Option 2: B0 = 12288, -6144 <= w <= 6144\n\n");
  ct_iteration(12288, -6144, 6144);
  printf("\n");

  printf("\nBase GS iterations\n\n");
  printf("Option 1: B0 = 12288, 0 <= w <= 12288\n\n");
  gs_iteration(12288, 0, 12288);
  printf("\n");

  printf("Option 2: B0 = 12288, -6144 <= w <= 6144\n\n");
  gs_iteration(12288, -6144, 6144);
  printf("\n");

  printf("\nBounds on red of products\n\n");
  printf("Option 1:\n  0 <= x <= 12288, 0 <= y <= 12288:\n");
  min = min_red_mul_interval(0, 12288, 0, 12288, &min_x, &min_y);
  max = max_red_mul_interval(0, 12288, 0, 12288, &max_x, &max_y);
  printf("  min red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", min, min_x, min_y);
  printf("  max red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", max, max_x, max_y);
  printf("\n");
  if (min < 0) min = -min;
  if (max < 0) max = -max;
  b = (min < max) ? max : min;
  ct_iteration(b, 0, 12288);
  gs_iteration(b, 0, 12288);

  printf("Option 2:\n  -6144 <= x <= 6144, -6144 <= y <= 6144:\n");
  min = min_red_mul_interval(-6144, 6144, -6144, 6144, &min_x, &min_y);
  max = max_red_mul_interval(-6144, 6144, -6144, 6144, &max_x, &max_y);
  printf("  min red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", min, min_x, min_y);
  printf("  max red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", max, max_x, max_y);
  printf("\n");
  if (min < 0) min = -min;
  if (max < 0) max = -max;
  b = (min < max) ? max : min;
  ct_iteration(b, -6144, 6144);
  gs_iteration(b, -6144, 6144);

  printf("Option 3:\n  0 <= x <= 12288, -6144 <= y <= 6144:\n");
  min = min_red_mul_interval(0, 12288, -6144, 6144, &min_x, &min_y);
  max = max_red_mul_interval(0, 12288, -6144, 6144, &max_x, &max_y);
  printf("  min red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", min, min_x, min_y);
  printf("  max red(x * y) = %"PRId64" (for x=%"PRId64", y=%"PRId64")\n", max, max_x, max_y);
  printf("\n");
  if (min < 0) min = -min;
  if (max < 0) max = -max;
  b = (min < max) ? max : min;
  ct_iteration(b, -6144, 6144);
  gs_iteration(b, -6144, 6144);


  show_ct_bounds("ntt16_red_ct_rev2std", 16, shoup_sred_ntt16_12289);
  show_ct_bounds("ntt16_red_ct_std2rev", 16, rev_shoup_sred_ntt16_12289);
  show_ct_bounds("mulntt16_red_ct_rev2std", 16, shoup_sred_scaled_ntt16_12289);
  show_ct_bounds("mulntt16_red_ct_std2rev", 16, rev_shoup_sred_scaled_ntt16_12289);

  show_ct_bounds("ntt128_red_ct_rev2std", 128, shoup_sred_ntt128_12289);
  show_ct_bounds("ntt128_red_ct_std2rev", 128, rev_shoup_sred_ntt128_12289);
  show_ct_bounds("mulntt128_red_ct_rev2std", 128, shoup_sred_scaled_ntt128_12289);
  show_ct_bounds("mulntt128_red_ct_std2rev", 128, rev_shoup_sred_scaled_ntt128_12289);

  show_ct_bounds("ntt256_red_ct_rev2std", 256, shoup_sred_ntt256_12289);
  show_ct_bounds("ntt256_red_ct_std2rev", 256, rev_shoup_sred_ntt256_12289);
  show_ct_bounds("mulntt256_red_ct_rev2std", 256, shoup_sred_scaled_ntt256_12289);
  show_ct_bounds("mulntt256_red_ct_std2rev", 256, rev_shoup_sred_scaled_ntt256_12289);

  show_ct_bounds("ntt512_red_ct_rev2std", 512, shoup_sred_ntt512_12289);
  show_ct_bounds("ntt512_red_ct_std2rev", 512, rev_shoup_sred_ntt512_12289);
  show_ct_bounds("mulntt512_red_ct_rev2std", 512, shoup_sred_scaled_ntt512_12289);
  show_ct_bounds("mulntt512_red_ct_std2rev", 512, rev_shoup_sred_scaled_ntt512_12289);

  show_ct_bounds("ntt1024_red_ct_rev2std", 1024, shoup_sred_ntt1024_12289);
  show_ct_bounds("ntt1024_red_ct_std2rev", 1024, rev_shoup_sred_ntt1024_12289);
  show_ct_bounds("mulntt1024_red_ct_rev2std", 1024, shoup_sred_scaled_ntt1024_12289);
  show_ct_bounds("mulntt1024_red_ct_std2rev", 1024, rev_shoup_sred_scaled_ntt1024_12289);

  show_ct_bounds("ntt2048_red_ct_rev2std", 2048, shoup_sred_ntt2048_12289);
  show_ct_bounds("ntt2048_red_ct_std2rev", 2048, rev_shoup_sred_ntt2048_12289);
  show_ct_bounds("mulntt2048_red_ct_rev2std", 2048, shoup_sred_scaled_ntt2048_12289);
  show_ct_bounds("mulntt2048_red_ct_std2rev", 2048, rev_shoup_sred_scaled_ntt2048_12289);

  return 0;
}


