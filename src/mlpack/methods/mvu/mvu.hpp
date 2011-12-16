/**
 * @file mvu.hpp
 * @author Ryan Curtin
 *
 * An implementation of Maximum Variance Unfolding.  This file defines an MVU
 * class as well as a class representing the objective function (a semidefinite
 * program) which MVU seeks to minimize.  Minimization is performed by the
 * Augmented Lagrangian optimizer (which in turn uses the L-BFGS optimizer).
 */
#ifndef __MLPACK_METHODS_MVU_MVU_HPP
#define __MLPACK_METHODS_VU_MVU_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace mvu {

/**
 * The MVU class is meant to provide a good abstraction for users.  The dataset
 * needs to be provided, as well as several parameters.
 *
 * - dataset
 * - new dimensionality
 */
class MVU
{
 public:
  MVU(const arma::mat& dataIn);

  void Unfold(const size_t newDim,
              const size_t numNeighbors,
              arma::mat& outputCoordinates);

 private:
  const arma::mat& data;
};

}; // namespace mvu
}; // namespace mlpack

#endif
