/*
 * Bounds on the reduction function
 */

#ifndef __RED_BOUNDS_H
#define __RED_BOUNDS_H

#include <stdint.h>

/*
 * Maximum of red(x) for a <= x <= b
 * - red(x) is returned, x is stored in *m
 */
extern int64_t max_red(int64_t a, int64_t b, int64_t *m);

/*
 * Minimum of red(x) for a <= x <= b
 */
extern int64_t min_red(int64_t a, int64_t b, int64_t *m);

/*
 * Maximum of red(w x) for a <= x <= b
 */
extern int64_t max_red_mul(int64_t a, int64_t b, int64_t w, int64_t *m);

/*
 * Minimum of red(w*x) for a <= x <= b
 */
extern int64_t min_red_mul(int64_t a, int64_t b, int64_t w, int64_t *m);

/*
 * Maximum of red(w * x) for a <= x <= b and low <= w <= high. 
 * - the max is returned. The corresponding x and w are stored in *m and
 * *mw, respectively.
 */
extern int64_t max_red_mul_interval(int64_t a, int64_t b, int64_t low, int64_t high, int64_t *m, int64_t *mw);
/*
 * Minimum of red(x * w) for a <= x <= b and low <= w <= high
 */
extern int64_t min_red_mul_interval(int64_t a, int64_t b, int64_t low, int64_t high, int64_t *m, int64_t *wm);


/*
 * Bounds after a CT step
 * - assuming |x| <= b and |y| <= b, this function returns b' 
 *   such that |x'| <= b' and |y'| <= b'  after executing
 *      x' = x + red(w * y)
 *      y' = x - red(w * y)
 *  for the worst-case w in interval [low, high].
 */
extern int64_t ct_bound(int64_t b, int64_t low, int64_t high);

/*
 * Bounds after a GS step: same as CT but the updates are
 *  x' = x + y
 *  y' = (x - y) * w.
 */
extern int64_t gs_bound(int64_t b, int64_t low, int64_t high);


/*
 * Bounds after a CT step with a fixed w
 * - assuming |x| <= b and |y| <= b, returns b' such that
 *    |x + red(w, y)| <= b' and |x - red(w, y)| <= b'
 */
extern int64_t ct_bound_fixed(int64_t b, int64_t w);

/*
 * Bounds after a GS step with a fixed w
 * - assuming |x| <= b and |y| <= b, returns b' such that
 *    |x + y| <= b' and |(x - y) * w| <= b'
 */
extern int64_t gs_bound_fixed(int64_t b, int64_t w);


/*
 * Bounds after ntt computations based on Cooley Tukey
 * - b0 = bound on the input
 * - p = array of coefficients used in the algorithm
 *   p[t + i] = omega^(n/2t)^i (or a variant of this).
 * We assume the input coefficients a[i] satisfy |a[i]| <= b0.
 *
 * The final bound is returned.
 * Bounds for each round are stored in array bounds (must be of size log_2(n))
 */
extern int64_t ntt_ct_bounds(int64_t b0, uint32_t n, const int16_t *p, int64_t *bound);

/*
 * Bounds after ntt computations based on Gentleman Sande
 * - b0 = bound on the input
 * - p = array of coefficients used in the algorithm
 *   p[t + i] = omega^(n/2t)^i (or a variant of this).
 * We assume the input coefficients a[i] satisfy |a[i]| <= b0.
 *
 * The final bound is returned.
 * Bounds for each round are stored in array bound (must be of size log_2(n))
 */
extern int64_t ntt_gs_bounds(int64_t b0, uint32_t n, const int16_t *p, int64_t *bound);

#endif /* __RED_BOUNDS_H */
