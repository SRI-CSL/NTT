/*
 * Interval abstract domain
 * - we represent intervals as pairs of 64bit signed numbers,
 *   which should be safe for our NTT
 */

#ifndef INTERVALS_H
#define INTERVALS_H

#include <stdint.h>

typedef struct interval_s {
  int64_t min;
  int64_t max;
} interval_t;


/*
 * Constructors: all allocate and return a pointer to an interval
 * structure.
 */
extern interval_t *point(int64_t x);
extern interval_t *interval(int64_t min, int64_t max);

/*
 * Destructor: just calls free
 */
extern void delete_interval(interval_t *a);

/*
 * Basic operations
 */
extern interval_t *add(const interval_t *a, const interval_t *b);
extern interval_t *sub(const interval_t *a, const interval_t *b);
extern interval_t *neg(const interval_t *a);

/*
 * Reductions
 * - red(a) = [l, h] such that l <= red(x) <= h for any x in a
 * - red_mul(a, b) = [l, h] such that l <= red(x * y) <= h for any x in a and y in b.
 * - red_scale(k, a) = [l, h] such that l <= red(x * k) <= h for any x in a
 * - red_twice(a) = [l, h] such that l <= red(red(x)) <= h for x in a.
 */
extern interval_t *red(const interval_t *a);
extern interval_t *red_mul(const interval_t *a, const interval_t *b);
extern interval_t *red_scale(int64_t k, const interval_t *a);
extern interval_t *red_twice(const interval_t *a);

/*
 * Reduction modulo q: [l, h] such that l <= x % q <= h whenever x is in a.
 * The modulo operation returns an integer between 0 and  q-1 here.
 * - q is 12289.
 */
extern interval_t *normal(const interval_t *a);

/*
 * Multiply by inverse(3) then reduce modulo q
 */
extern interval_t *normal_inv3(const interval_t *a);

/*
 * Shift representation: a must be a sub-interval of [0 .. q-1]
 * - returns [l, h] such that l <= shift(x) <= h where
 *   shift(x) = x if 0 <= x <= (q-1)/2
 *   shift(x) = x - q if (q-1/2) < x <= q-1
 */
extern interval_t *shift(const interval_t *a);


/*
 * Correct: assume x is in the interval [-q, 2*q-1] then
 * correct(x) is if (x<0) then x+q elsif (x >= q) then x-q else x.
 * So correct(x) is in thee interval [0 .. q-1]
 *
 * Interval a must be a subinterval of [-q, 2q-1]
 * Correct(a) returns [l, h] such that l <= correct(x) <= h for x in a.
 */
extern interval_t *correct(const interval_t *a);


#endif /* INTERVALS_H */
