/**
 * @file clamp.hpp
 *
 * Miscellaneous math clamping routines.
 */
#ifndef __MLPACK_CORE_MATH_CLAMP_HPP
#define __MLPACK_CORE_MATH_CLAMP_HPP

#include <stdlib.h>
#include <math.h>
#include <float.h>

namespace mlpack {
namespace math /** Miscellaneous math routines. */ {

/**
 * Forces a number to be non-negative, turning negative numbers into zero.
 * Avoids branching costs (this is a measurable improvement).
 *
 * @param d Double to clamp.
 * @return 0 if d < 0, d otherwise.
 */
inline double ClampNonNegative(double d)
{
  return (d + fabs(d)) / 2;
}

/**
 * Forces a number to be non-positive, turning positive numbers into zero.
 * Avoids branching costs (this is a measurable improvement).
 *
 * @param d Double to clamp.
 * @param 0 if d > 0, d otherwise.
 */
inline double ClampNonPositive(double d)
{
  return (d - fabs(d)) / 2;
}

/**
 * Clamp a number between a particular range.
 *
 * @param value The number to clamp.
 * @param rangeMin The first of the range.
 * @param rangeMax The last of the range.
 * @return max(rangeMin, min(rangeMax, d)).
 */
inline double ClampRange(double value, double rangeMin, double rangeMax)
{
  value -= rangeMax;
  value = ClampNonPositive (value) + rangeMax;
  value -= rangeMin;
  value = ClampNonNegative (value) + rangeMin;
  return value;
}

}; // namespace math
}; // namespace mlpack

#endif // __MLPACK_CORE_MATH_CLAMP_HPP
