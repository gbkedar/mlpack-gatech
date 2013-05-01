/**
 * @file random_init.hpp
 * @author Mohan Rajendran
 *
 * Intialization rule for Non-Negative Matrix Factorization (NMF). This simple
 * initialization is performed by assigning a random matrix to W and H.
 */
#ifndef __MLPACK_METHODS_NMF_RANDOM_INIT_HPP
#define __MLPACK_METHODS_NMF_RANDOM_INIT_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace nmf {

class RandomInitialization
{
 public:
  // Empty constructor required for the InitializeRule template
  RandomInitialization() { }

  inline static void Initialize(const arma::mat& V,
                                const size_t r,
                                arma::mat& W,
                                arma::mat& H)
  {
    // Simple implementation (left in the header file due to its simplicity).
    size_t n = V.n_rows;
    size_t m = V.n_cols;

    // Intialize to random values.
    W.randu(n, r);
    H.randu(r, m);
  }
};

}; // namespace nmf
}; // namespace mlpack

#endif
