/*
 * Interval abstract domain
 * - we represent intervals as pairs of 64bit signed numbers,
 *   which should be safe for our NTT
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include "intervals.h"
#include "red_bounds.h"

/*
 * Malloc wrapper
 */
static interval_t *new_interval(void) {
  interval_t *a;

  a = malloc(sizeof(interval_t));
  if (a == NULL) {
    fprintf(stderr, "Out of memory: failed to allocate interval descriptor\n");
    exit(1);
  }
  return a;
}

/*
 * Destructor: just calls free
 */
void delete_interval(interval_t *a) {
  free(a);
}

/*
 * Create interval [x, y]
 */
static interval_t *make_interval(int64_t x, int64_t y) {
  interval_t *a;

  assert(x <= y);
  a = new_interval();
  a->min = x;
  a->max = y;
  return a;
}

/*
 * Point interval
 */
interval_t *point(int64_t x) {
  return make_interval(x, x);
}

/*
 * Interval [min, max]: fails if min > max
 */
interval_t *interval(int64_t min, int64_t max) {
  if (min > max) {
    fprintf(stderr, "Bad interval: [%"PRId64", %"PRId64"]\n", min, max);
    abort();
  }

  return make_interval(min, max);
}


/*
 * Negate: [l, h] --> [-h, -l] (check for overflow)
 */
interval_t *neg(const interval_t *a) {
  if (a->min == INT64_MIN) {
    fprintf(stderr, "Overflow: can't negate interval: [%"PRId64", %"PRId64"]\n", a->min, a->max);
    exit(1);
  }
  return make_interval(- a->max, - a->min);
}

// addition of x and y with overflow detection
static int64_t add64(int64_t x, int64_t y) {
  int64_t z;

  z = x + y;
  if ((x < 0 && y < 0 && z >= 0) || (x >= 0 && y >= 0 && z < 0)) {
    fprintf(stderr, "Arithmetic overflow in %"PRId64" + %"PRId64"\n", x, y);
    exit(1);
  }
  return z;
}

// subtraction: x - y with overflow detection
static int64_t sub64(int64_t x, int64_t y) {
  int64_t z;

  z = x - y;
  if ((x < 0 && y >= 0 && z >= 0) || (x >= 0 && y < 0 && z < 0)) {
    fprintf(stderr, "Arithmetic overflow in %"PRId64" - %"PRId64"\n", x, y);
    exit(1);
  }
  return z;
}


/*
 * Size of interval a
 */
static uint64_t size(const interval_t *a) {
  assert(a->min <= a->max);
  // assuming 2s complement, this gives the right answer even
  // if there's an overflow.
  return (uint64_t) (a->max - a->min);
}

/*
 * Add: [l1, h1] + [l2, h2] = [l1+l2, h1+h2]
 */
interval_t *add(const interval_t *a, const interval_t *b) {
  return make_interval(add64(a->min, b->min), add64(a->max, b->max));
}

/*
 * Sub: [l1, h1] - [l2, h2] = [l1-h2, h1-l2]
 */
interval_t *sub(const interval_t *a, const interval_t *b) {
  return make_interval(sub64(a->min, b->max), sub64(a->max, b->min));
}

/*
 * Interval for red(a): [l, h] such that l <= red(x) <= h whenever x in a
 */
interval_t *red(const interval_t *a) {
  int64_t x, y, min, max;

  min = min_red(a->min, a->max, &x);
  max = max_red(a->min, a->max, &y);
  return make_interval(min, max);
}


/*
 * Interval for red_twice(a): [l, h] such that l <= red(red(x)) <= h whenever x in a
 */
interval_t *red_twice(const interval_t *a) {
  int64_t x, y, min0, max0, min, max;

  min0 = min_red(a->min, a->max, &x);
  max0 = max_red(a->min, a->max, &y);
  assert(min0 <= max0);
  min = min_red(min0, max0, &x);
  max = max_red(min0, max0, &y);
  return make_interval(min, max);
}


/*
 * Interval for red_mul(a, b): [l, h] such that l <= red(x * y) <= h when
 * x is in a and y is in b.
 */
interval_t *red_mul(const interval_t *a, const interval_t *b) {
  int64_t x1, x2, y1, y2, min, max;
  const interval_t *aux;

  /*
   * The min/max_red_mul_interval functions compute
   * min(red(x * w)) and max(red(x * w)) by iteration for w in an interval [low, high].
   * It's OK if the interval for x is large, but we want the interval for w to be small,
   * otherwise the computation takes forever.
   */
  if (size(a) < size(b)) {
    // swap a and b
    aux = a; a = b; b = aux;
  }

  min = min_red_mul_interval(a->min, a->max, b->min, b->max, &x1, &x2);
  max = max_red_mul_interval(a->min, a->max, b->min, b->max, &y1, &y2);
  return make_interval(min, max);
}


/*
 * Interval for red_scale(k, a): [l, h] such that l <= red(x * k) <= h
 * when x is in a.
 */
interval_t *red_scale(int64_t k, const interval_t *a) {
  int64_t x, y, min, max;

  min = min_red_mul(a->min, a->max, k, &x);
  max = max_red_mul(a->min, a->max, k, &y);
  return make_interval(min, max);
}


/*
 * Remainder modulo Q
 */
static int64_t rem_q(int64_t x) {
  int64_t r;

  r = x % 12289;
  if (r < 0) r += 12289;
  assert(0 <= r && r <= 12288);
  return r;
}

/*
 * Reduction modulo q: [l, h] such that l <= x % q <= h whenever x is in a.
 * The modulo operation returns an integer between 0 and  q-1 here.
 * - q is 12289.
 */
interval_t *normal(const interval_t *a) {
  int64_t l, h;

  if (size(a) >= 12288) {
    return make_interval(0, 12288);
  }

  l = rem_q(a->min);
  h = rem_q(a->max);

  if (l <= h) {
    return make_interval(l, h);
  } else {
    return make_interval(0, 12288);
  }
}


/*
 * Multiply by inverse(3) then reduce modulo q
 * - we don't try to be precise unless q is a point interval
 */
interval_t *normal_inv3(const interval_t *a) {
  int64_t x;

  if (a->min == a->max) {
    x = rem_q(rem_q(a->min) * 8193);
    return make_interval(x, x);
  } else {
    return make_interval(0, 12288);
  }
}

/*
 * Shift representation: a must be a sub-interval of [0 .. q-1]
 * - returns [l, h] such that l <= shift(x) <= h where
 *   shift(x) = x if 0 <= x <= (q-1)/2
 *   shift(x) = x - q if (q-1/2) < x <= q-1
 */
interval_t *shift(const interval_t *a) {
  assert(0 <= a->min && a->min <= a->max && a->max <= 12288);
  if (a->min > 6144) {
    return make_interval(a->min - 12289, a->max - 12289);
  } else if (a->max <= 6144) {
    return make_interval(a->min, a->max);
  } else {
    return make_interval(-6144, +6144);
  }
}
