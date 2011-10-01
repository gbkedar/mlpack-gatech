/**
 * @file math_lib.h
 *
 * Includes all basic FASTlib non-vector math utilities.
 */

#ifndef MATH_MATH_LIB_H
#define MATH_MATH_LIB_H

#include "fastlib/fx/io.h"

#include <math.h>
#include <float.h>
/**
 * Math routines.
 *
 * The hope is that this should contain most of the useful math routines
 * you can think of.  Currently, this is very sparse.
 */
namespace math {
  /** The square root of 2. */
  const double SQRT2 = 1.41421356237309504880;
  /** Base of the natural logarithm. */
  const double E = 2.7182818284590452354;
  /** Log base 2 of E. */
  const double LOG2_E = 1.4426950408889634074;
  /** Log base 10 of E. */
  const double LOG10_E = 0.43429448190325182765;
  /** Natural log of 2. */
  const double LN_2 = 0.69314718055994530942;
  /** Natural log of 10. */
  const double LN_10 = 2.30258509299404568402;
  /** The ratio of the circumference of a circle to its diameter. */
  const double PI = 3.141592653589793238462643383279;
  /** The ratio of the circumference of a circle to its radius. */
  const double PI_2 = 1.57079632679489661923;

  /** Squares a number. */
  template<typename T>
  inline T Sqr(T v) {
    return v * v;
  }

  /**
   * Rounds a double-precision to an integer, casting it too.
   */
  inline int64_t RoundInt(double d) {
    return int64_t(nearbyint(d));
  }

  /**
   * Forces a number to be non-negative, turning negative numbers into zero.
   *
   * Avoids branching costs (yes, we've discovered measurable improvements).
   */
  inline double ClampNonNegative(double d) {
    return (d + fabs(d)) / 2;
  }

  /**
   * Forces a number to be non-positive, turning positive numbers into zero.
   *
   * Avoids branching costs (yes, we've discovered measurable improvements).
   */
  inline double ClampNonPositive(double d) {
    return (d - fabs(d)) / 2;
  }

  /**
   * Clips a number between a particular range.
   *
   * @param value the number to clip
   * @param range_min the first of the range
   * @param range_max the last of the range
   * @return max(range_min, min(range_max, d))
   */
  inline double ClampRange(double value, double range_min, double range_max) {
    if (value <= range_min) {
      return range_min;
    } else if (value >= range_max) {
      return range_max;
    } else {
      return value;
    }
  }

  /**
   * Generates a uniform random number between 0 and 1.
   */
  inline double Random() {
    return rand() * (1.0 / RAND_MAX);
  }

  /**
   * Generates a uniform random number in the specified range.
   */
  inline double Random(double lo, double hi) {
    return Random() * (hi - lo) + lo;
  }

  /**
   * Generates a uniform random integer.
   */
  inline int RandInt(int hi_exclusive) {
    return rand() % hi_exclusive;
  }
  /**
   * Generates a uniform random integer.
   */
  inline int RandInt(int lo, int hi_exclusive) {
    return (rand() % (hi_exclusive - lo)) + lo;
  }
};

#include "math_lib_impl.h"
//#include "math_lib_impl.h"

namespace math {
  /**
   * Calculates a relatively small power using template metaprogramming.
   *
   * This allows a numerator and denominator.  In the case where the
   * numerator and denominator are equal, this will not do anything, or in
   * the case where the denominator is one.
   */
  template<int t_numerator, int t_denominator>
  inline double Pow(double d) {
    return math__private::ZPowImpl<t_numerator, t_denominator>::Calculate(d);
  }

  /**
   * Calculates a small power of the absolute value of a number
   * using template metaprogramming.
   *
   * This allows a numerator and denominator.  In the case where the
   * numerator and denominator are equal, this will not do anything, or in
   * the case where the denominator is one.  For even powers, this will
   * avoid calling the absolute value function.
   */
  template<int t_numerator, int t_denominator>
  inline double PowAbs(double d) {
    // we specify whether it's an even function -- if so, we can sometimes
    // avoid the absolute value sign
    return math__private::ZPowAbsImpl<t_numerator, t_denominator,
        (t_numerator%t_denominator == 0) && ((t_numerator/t_denominator)%2 == 0)>::Calculate(fabs(d));
  }
};

/**
 * A value which is the min or max of multiple other values.
 *
 * Comes with a highly optimized version of x = max(x, y).
 *
 * The template argument should be something like double, with greater-than,
 * less-than, and equals operators.
 */
template<typename TValue>
class MinMaxVal {
 public:
  typedef TValue Value;

 public:
  /** The underlying value. */
  Value val;

 public:
  /**
   * Converts implicitly to the value.
   */
  operator Value() const { return val; }

  /**
   * Sets the value.
   */
  const Value& operator = (Value val_in) {
    return (val = val_in);
  }

  /**
   * Efficiently performs this->val = min(this->val, incoming_val).
   *
   * The expectation is that it is higly unlikely for the incoming
   * value to be the new minimum.
   */
  void MinWith(Value incoming_val) {
    if (incoming_val < val) {
      val = incoming_val;
    }
  }

  /**
   * Efficiently performs this->val = min(this->val, incoming_val).
   *
   * The expectation is that it is higly unlikely for the incoming
   * value to be the new maximum.
   */
  void MaxWith(Value incoming_val) {
    if (incoming_val > val) {
      val = incoming_val;
    }
  }
};

#include "range.h"
#include "kernel.h"

#endif
