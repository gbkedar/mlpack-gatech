/**
 * @file tree/periodichrectbound.h
 *
 * Bounds that are useful for binary space partitioning trees.
 *
 * This file describes the interface for the PeriodicHRectBound policy, which
 * implements a hyperrectangle bound in a periodic space.
 */
#ifndef __MLPACK_CORE_TREE_PERCLIDICHRECTBOUND_HPP
#define __MLPACK_CORE_TREE_PERCLIDICHRECTBOUND_HPP

#include <mlpack/core.h>

namespace mlpack {
namespace bound {

/**
 * Hyper-rectangle bound for an L-metric.
 *
 * Template parameter t_pow is the metric to use; use 2 for Euclidean (L2).
 */
template<int t_pow = 2>
class PeriodicHRectBound
{
 public:
  /**
   * Empty constructor.
   */
  PeriodicHRectBound();

  /**
   * Specifies the box size.  The dimensionality is set to the same of the box
   * size, and the bounds are initialized to be empty.
   */
  PeriodicHRectBound(arma::vec box);

  /***
   * Copy constructor and copy operator.  These are necessary because we do our
   * own memory management.
   */
  PeriodicHRectBound(const PeriodicHRectBound& other);
  PeriodicHRectBound& operator=(const PeriodicHRectBound& other);

  /**
   * Destructor: clean up memory.
   */
  ~PeriodicHRectBound();

  /**
   * Modifies the box_size_ to the desired dimenstions.
   */
  void SetBoxSize(arma::vec box);

  /**
   * Returns the box_size_ vector.
   */
  const arma::vec& box() const { return box_; }

  /**
   * Resets all dimensions to the empty set.
   */
  void Clear();

  /** Gets the dimensionality */
  size_t dim() const { return dim_; }

  /**
   * Sets and gets the range for a particular dimension.
   */
  math::Range& operator[](size_t i);
  const math::Range operator[](size_t i) const;

  /***
   * Calculates the centroid of the range.  This does not factor in periodic
   * coordinates, so the centroid may not necessarily be inside the given box.
   *
   * @param centroid Vector to write the centroid to.
   */
  void Centroid(arma::vec& centroid) const;

  /**
   * Calculates minimum bound-to-point squared distance in the periodic bound
   * case.
   */
  double MinDistance(const arma::vec& point) const;

  /**
   * Calculates minimum bound-to-bound squared distance in the periodic bound
   * case.
   *
   * Example: bound1.MinDistance(other) for minimum squared distance.
   */
  double MinDistance(const PeriodicHRectBound& other) const;

  /**
   * Calculates maximum bound-to-point squared distance in the periodic bound
   * case.
   */
  double MaxDistance(const arma::vec& point) const;

  /**
   * Computes maximum bound-to-bound squared distance in the periodic bound
   * case.
   */
  double MaxDistance(const PeriodicHRectBound& other) const;

  /**
   * Calculates minimum and maximum bound-to-point squared distance in the
   * periodic bound case.
   */
  math::Range RangeDistance(const arma::vec& point) const;

  /**
   * Calculates minimum and maximum bound-to-bound squared distance in the
   * periodic bound case.
   */
  math::Range RangeDistance(const PeriodicHRectBound& other) const;

  /**
   * Expands this region to include a new point.
   */
  PeriodicHRectBound& operator|=(const arma::vec& vector);

  /**
   * Expands this region to encompass another bound.
   */
  PeriodicHRectBound& operator|=(const PeriodicHRectBound& other);

  /**
   * Determines if a point is within this bound.
   */
  bool Contains(const arma::vec& point) const;

 private:
  math::Range *bounds_;
  size_t dim_;
  arma::vec box_;
};

}; // namespace bound
}; // namespace mlpack

#include "periodichrectbound_impl.hpp"

#endif // __MLPACK_CORE_TREE_PERCLIDICHRECTBOUND_HPP
