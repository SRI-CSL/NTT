#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#include "red_bounds.h"

/*
 * 12289 is 3 * 2^12 + 1
 * MASK = 2^12 - 1
 */
#define Q 12289
#define K 3
#define MASK 4095


static int64_t red(int64_t x) {
  return (3 * (x & 4095)) - (x >> 12);
}

static int64_t divd(int64_t x) {
  return x >> 12;
}

static int64_t remd(int64_t x) {
  return x & 4095;
}


/*
 * BOUNDS ON RED(x)
 */

/*
 * Maximum of red(x) for a <= x <= b
 * - red(x) is returned, x is stored in *m
 */
int64_t max_red(int64_t a, int64_t b, int64_t *m) {
  int64_t d, r;
  
  assert(a <= b);

  d = a | 4095;
  d = (d <= b) ? d : b;
  r = red(d);
  *m = d;

  return r;
}

/*
 * Minimum of red(x) for a <= x <= b
 */
int64_t min_red(int64_t a, int64_t b, int64_t *m) {
  int64_t d, r;

  assert(a <= b);

  d = b & ~4095;
  d = (d >= a) ? d : a;
  r = red(d);
  *m = d;
  return r;
}



/*
 * BOUNDS ON RED(w * x)
 */

/*
 * GCD of w and 4096
 */
static int64_t gcd4096(int64_t w) {
  int64_t g;

  g = 1;
  while ((w & 1) == 0) {
    g <<= 1;
    w >>= 1;
  }
  // g is the largest power of two that divides w
  return (g < 4096) ? g : 4096;
}

/*
 * Largest y such that (w x)>>12 == (w y)>>12 are equal.
 * So, for any z such that x <= z <= y, we have red(w y) >= red(w z).
 */
static int64_t lmax(int64_t w, int64_t x) {
  int64_t y, k;

  k = (4095 - ((w * x) & 4095))/w;  // floor((4096 - 1 - r0)/w) where r0 = (w * x) & 4095
  y =  x + k;
  assert((w * y) >> 12 == (w * x) >> 12);
  assert((w * (y+1)) >> 12 > (w * x) >> 12);

  return y;
}


/*
 * Maximum of red(w x) for a <= x <= b
 */
int64_t max_red_mul(int64_t a, int64_t b, int64_t w, int64_t *m) {
  int64_t pw, g, h, x, r, x_max, r_max;
  
  assert(a <= b);

  // pw = |w|. 
  // If w<0, we change a <= x <= b to -b <= -x <= -a
  pw = w;
  if (w < 0) {
    x = -a; a = -b; b = x;
    pw = -w;
  }

  x_max = b;
  r_max = red(pw * b);

  // The remainder of (w * x) by 4096 is a multiple of gcd(4096, w)
  // so it's at most h = 4096 - gcd(4096, w).
  // We then have red(w * x) <= - divd(w * x) + g,
  h = 4096 - gcd4096(w);
  g = 3 * h;

  x = a;
  for (;;) {
    x = lmax(pw, x);
    if (x >= b || - divd(pw * x) + g <= r_max) break;
    r = red(pw * x);
    if (r > r_max) {
      x_max = x;
      r_max = r;
      if (remd(pw * x) == h) {
	// this is the largest possible remainder
	// so r_max >= red(pw * y) when y >= x.
	break;
      }
    }
    x ++;
  }

  if (w < 0) x_max = - x_max;

  *m = x_max;
  return r_max;
}


/*
 * Smallest y such that (w x) >> 12 == (w y) >> 12.
 * For any z such that y <= z <= x, we have red(w y) <= red(w z).
 */
static int64_t lmin(int64_t w, int64_t x) {
  int64_t y, k;

  k = ((w * x) & 4095)/w;
  y = x - k;
  assert((w * y) >> 12 == (w * x) >> 12);
  assert((w * (y - 1)) >> 12 < (w * x) >> 12);

  return y;
}

/*
 * Minimum of red(w*x) for a <= x <= b
 */
int64_t min_red_mul(int64_t a, int64_t b, int64_t w, int64_t *m) {
  int64_t pw, x, r, x_min, r_min;
  
  assert(a <= b);

  // pw = |w|. 
  // If w<0, we change a <= x <= b to -b <= -x <= -a
  pw = w;
  if (w < 0) {
    x = -a; a = -b; b = x;
    pw = -w;
  }

  x_min = a;
  r_min = red(pw * a);

  x = b;
  for (;;) {
    x = lmin(pw, x);
    if (x <= a || -divd(pw * x) >= r_min) break;
    r = red(pw * x);
    if (r < r_min) {
      x_min = x;
      r_min = r;
      if (remd(pw * x) == 0) {	
	break;
      }
    }
    x --;
  }

  if (w < 0) x_min = - x_min;

  *m = x_min;
  return r_min;
}



/*
 * Maximum of red(w * x) for a <= x <= b and low <= w <= high.
 */
int64_t max_red_mul_interval(int64_t a, int64_t b, int64_t low, int64_t high, int64_t *m, int64_t *mw) {
  int64_t r_max, x_max, w_max, r, x, w, d, l, h;

  assert(a <= b && low <= high);

  r_max = 0;
  x_max = 0;
  w_max = 0;

  // negative values of w: l <= w < h
  l = low < 0 ? low : 0;
  h = high >= 0 ? 0 : high + 1;
  for (w = l; w < h;  w++) {
    assert(w < 0);

    // d = min (w*x)/2^m for |x| <= b
    // so red(w*x) <= -d + 3*4095 for |x| <= b
    // also for any w' such that w <= w' < 0
    d = divd(w * b);
    if ( -d + 3 * 4095 <= r_max) {
      break;
    }
    r = max_red_mul(a, b, w, &x);
    if (r > r_max) {
      r_max = r;
      x_max = x;
      w_max = w;
    }
  }

  // positive values of w: l < w <= h
  l = low <= 0 ? 0 : low - 1;
  h = high > 0 ? high : 0;
  for (w = h; w > l; w --) {
    assert(w > 0);

    // min of (w*x)/2^m is -(w * b)/2^m
    d = divd(- w * b);
    if ( -d + 3 * 4095 <= r_max) {
      break;
    }
    r = max_red_mul(a, b, w, &x);
    if (r > r_max) {
      r_max = r;
      x_max = x;
      w_max = w;
    }
  }

  *m = x_max;
  *mw = w_max;

  return r_max;
}

 /*
  * Minimum of red(x * w) for a <= x <= b and low <= w <= high
  */
int64_t min_red_mul_interval(int64_t a, int64_t b, int64_t low, int64_t high, int64_t *m, int64_t *wm) {
   int64_t r_min, x_min, w_min, r, x, w, d, l, h;

   assert(a <= b && low <= high);

   r_min = 0;
   x_min = 0;
   w_min = 0;

   // negative values of w
   l = low < 0 ? low : 0;
   h = high > 0 ? 0 : high + 1;
   for (w = l; w < h; w++) {
     assert(w < 0);
     // d = max of (w * b)/2^m
     // -b <= x ==> w * x <= -b w ==> divd(w * x) <= divd(-w * b) ==> - div(w * x) >= - d
     // ==> red(wx) >= -d
     d = divd(- w * b);
     if (-d >= r_min) {
       break;
     }
     r = min_red_mul(a, b, w, &x);
     if (r < r_min) {
       r_min = r;
       x_min = x;
       w_min = w;
     }
   }

   // positive values of w
   l = low < 0 ? 0 : low - 1;
   h = high > 0 ? high : 0;
   for (w=h; w>l; w--) {
     assert(w > 0);

     d = divd(w * b); // max of (w * b)/2^m for |x| <= b
     if (-d >= r_min) {
       break;
     }
     r = min_red_mul(a, b, w, &x);
     if (r < r_min) {
       r_min = r;
       x_min = x;
       w_min = w;
     }
   }

   *m = x_min;
   *wm = w_min;

   return r_min;
}


/*
 * BOUND INCREASE IN NTT ALGORITHMS
 */

/*
 * Bound after CT step
 * - we assume |x| <= b and |y| <= b
 * - we want b' such that |x'| <= b' and |y'| <= b'  after executing
 *      x' = x + red(w * y)
 *      y' = x - red(w * y)
 *  for the worst-case w in interval [low, high].
 */
int64_t ct_bound(int64_t b, int64_t low, int64_t high) {
  int64_t min_r, max_r, min_y, min_w, max_y, max_w, b1, b2;

  assert(b >= 0 && low <= high);

  // min and max of red(w * y)
  min_r = min_red_mul_interval(-b, b, low, high, &min_y, &min_w);
  max_r = max_red_mul_interval(-b, b, low, high, &max_y, &max_w);

  // we have -b + min_r <= x' <= b + max_r
  // and     -b - max_r <= y' <= b - min_r
  b1 = b + max_r;
  b2 = b - min_r;
  if (b1 < 0) b1 = -b1;
  if (b2 < 0) b2 = -b2;

  return (b1 < b2) ? b2 : b1;
}


/*
 * Bounds after GS step: same as CT but the updates are
 *  x' = x + y
 *  y' = (x - y) * w.
 */
int64_t gs_bound(int64_t b, int64_t low, int64_t high) {
  int64_t min_r, max_r, min_w, max_w, min_d, max_d, b1, b2;

  assert(b >= 0 && low <= high);

  // | x - y| <= 2b
  min_r = min_red_mul_interval(-2 * b, 2 * b, low, high, &min_d, &min_w);
  max_r = max_red_mul_interval(-2 * b, 2 * b, low, high, &max_d, &max_w);

  // we have min_r <= y' <= max_r and -2b <= x <= 2b 
  // b1 = bound on |y'|, b2 = boud on |x'|
  b1 = (max_r < 0) ? -min_r : (min_r > 0) ? max_r : (max_r > -min_r) ? max_r : -min_r;
  b2 = 2 * b;

  return (b1 < b2) ? b2 : b1;
}

/*
 * Bounds after a CT step with a fixed w
 */
int64_t ct_bound_fixed(int64_t b, int64_t w) {
  int64_t min_r, max_r, min_y, max_y, b1, b2;

  assert(b >= 0);

  // min and max or red(w * y) for -b <= y <= b
  min_r = min_red_mul(-b, b, w, &min_y);
  max_r = max_red_mul(-b, b, w, &max_y);

  // we have -b + min_r <= x' <= b + max_r
  // and     -b - max_r <= y' <= b - min_r
  b1 = b + max_r;
  b2 = b - min_r;
  if (b1 < 0) b1 = -b1;
  if (b2 < 0) b2 = -b2;

  return (b1 < b2) ? b2 : b1;
}


/*
 * Bounds after a GS step with a fixed w
 */
int64_t gs_bound_fixed(int64_t b, int64_t w) {
  int64_t min_r, max_r, min_y, max_y, b1, b2;

  assert(b >= 0);

  // min and max or red(w * (x -y)) given |x - y| <= 2b
  min_r = min_red_mul(-2*b, 2*b, w, &min_y);
  max_r = max_red_mul(-2*b, 2*b, w, &max_y);

  // we have min_r <= y' <= max_r and -2b <= x <= 2b 
  // b1 = bound on |y'|, b2 = boud on |x'|
  b1 = (max_r < 0) ? -min_r : (min_r > 0) ? max_r : (max_r > -min_r) ? max_r : -min_r;
  b2 = 2 * b;

  return (b1 < b2) ? b2 : b1;
}


/*
 * Bounds after ntt computations based on Cooley Tukey
 * - b0 = bound on the input
 * - p = array of coefficients used in the algorithm
 *   p[t + i] = omega^(n/2t)^i (or a variant of this).
 * We assume the input coefficients a[i] satisfy |a[i]| <= b0,
 */
int64_t ntt_ct_bounds(int64_t b0, uint32_t n, const int16_t *p, int64_t *bound) {
  uint32_t j, t, k;
  int64_t b, c, d;

  b = b0;
  k = 0;
  bound[k] = b0;
  for (t=1; t<n; t<<=1) {
    k ++;
    /*
     * Each iteration modifies array a[0 ... n-1] in place. For
     * i=0...n-1, we have |a[i]| <= b on entry to the iteration.
     *
     * c is the max of ct_bound for j=0 .. t-1
     */
    c = ct_bound_fixed(b, p[t]);
    for (j=1; j<t; j++) {
      d = ct_bound_fixed(b, p[t + j]);
      if (d > c) c = d;
    }
    bound[k] = c;
    b = c;
  }

  return b;
}


/*
 * Bounds after ntt computations based on Gentleman Sande
 * - b0 = bound on the input
 * - p = array of coefficients used in the algorithm
 *   p[t + i] = omega^(n/2t)^i (or a variant of this).
 * We assume the input coefficients a[i] satisfy |a[i]| <= b0,
 */
int64_t ntt_gs_bounds(int64_t b0, uint32_t n, const int16_t *p, int64_t *bound) {
  uint32_t t, j, k;
  int64_t b, c, d;

  b = b0;
  k = 0;
  bound[k] = b0;
  for (t=n/2; t>0; t>>=1) {
    k ++;
    c = gs_bound_fixed(b, p[t]);
    for (j=1; j<t; j++) {
      d = gs_bound_fixed(b, p[t + j]);
      if (d > c) c = d;
    }
    bound[k] = c;
    b = c;
  }

  return b;
}
