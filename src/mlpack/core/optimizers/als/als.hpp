/**
 * @file als.hpp
 * @author Mohan Rajendran
 * @author Mudit Raj Gupta
 *
 * Defines the ALS class for non-negative Matrix Factorization
 * on a given sparcely populated matrix.
 */
#ifndef __MLPACK_CORE_OPTIMIZERS_ALS_ALS_HPP
#define __MLPACK_CORE_OPTIMIZERS_ALS_ALS_HPP

#include <mlpack/core.hpp>
#include "als_update_rules.hpp"
#include "random_init.hpp"

namespace mlpack {
namespace als {

/**
 * This class implements the NMF on the given matrix V. Non-negative Matrix
 * Factorization decomposes V in the form \f$ V \approx WH \f$ where W is
 * called the basis matrix and H is called the encoding matrix. V is taken
 * to be of size n x m and the obtained W is n x r and H is r x m. The size r is
 * called the rank of the factorization.
 * 
 * For more information on non-negative matrix factorization, see the following
 * paper:
 *
 * @code
 * @article{
 *   title = {{Learning the parts of objects by non-negative matrix
 *       factorization}},
 *   author = {Lee, Daniel D. and Seung, H. Sebastian},
 *   journal = {Nature},
 *   month = {Oct},
 *   year = {1999},
 *   number = {6755},
 *   pages = {788--791},
 *   publisher = {Nature Publishing Group},
 *   url = {http://dx.doi.org/10.1038/44565}
 * }
 * @endcode
 *
 * @tparam WUpdateRule The update rule for calculating W matrix at each
 *     iteration; @see MultiplicativeDistanceW for an example.
 * @tparam HUpdateRule The update rule for calculating H matrix at each
 *     iteration; @see MultiplicativeDistanceH for an example.
 */
template<typename InitializationRule = RandomInitialization,
         typename WUpdateRule = WAlternatingLeastSquaresRule,
         typename HUpdateRule = HAlternatingLeastSquaresRule>
class ALS
{
 public:
  /**
   * Create the NMF object and (optionally) set the parameters which NMF will
   * run with.  The minimum residue refers to the root mean square of the
   * difference between two subsequent iterations of the product W * H.  A low
   * residue indicates that subsequent iterations are not producing much change
   * in W and H.  Once the residue goes below the specified minimum residue, the
   * algorithm terminates.
   *
   * @param maxIterations Maximum number of iterations allowed before giving up.
   *     A value of 0 indicates no limit.
   * @param minResidue The minimum allowed residue before the algorithm
   *     terminates.
   * @param Initialize Optional Initialization object for initializing the
   *     W and H matrices
   * @param WUpdate Optional WUpdateRule object; for when the update rule for
   *     the W vector has states that it needs to store.
   * @param HUpdate Optional HUpdateRule object; for when the update rule for
   *     the H vector has states that it needs to store.
   */
  ALS(const size_t maxIterations = 10000,
      const double minResidue = 1e-10,
      const InitializationRule initializeRule = InitializationRule(),
      const WUpdateRule wUpdate = WUpdateRule(),
      const HUpdateRule hUpdate = HUpdateRule());

  /**
   * Apply Non-Negative Matrix Factorization to the provided matrix.
   *
   * @param V Input matrix to be factorized.
   * @param W Basis matrix to be output.
   * @param H Encoding matrix to output.
   * @param r Rank r of the factorization.
   */
 void Apply(const arma::sp_mat& V, const size_t r, arma::mat& W, arma::mat& H)
      const;

 private:
  //! The maximum number of iterations allowed before giving up.
  size_t maxIterations;
  //! The minimum residue, below which iteration is considered converged.
  double minResidue;
  //! Instantiated initialization Rule.
  InitializationRule initializeRule;
  //! Instantiated W update rule.
  WUpdateRule wUpdate;
  //! Instantiated H update rule.
  HUpdateRule hUpdate;

 public:
  //! Access the maximum number of iterations.
  size_t MaxIterations() const { return maxIterations; }
  //! Modify the maximum number of iterations.
  size_t& MaxIterations() { return maxIterations; }
  //! Access the minimum residue before termination.
  double MinResidue() const { return minResidue; }
  //! Modify the minimum residue before termination.
  double& MinResidue() { return minResidue; }
  //! Access the initialization rule.
  const InitializationRule& InitializeRule() const { return initializeRule; }
  //! Modify the initialization rule.
  InitializationRule& InitializeRule() { return initializeRule; }
  //! Access the W update rule.
  const WUpdateRule& WUpdate() const { return wUpdate; }
  //! Modify the W update rule.
  WUpdateRule& WUpdate() { return wUpdate; }
  //! Access the H update rule.
  const HUpdateRule& HUpdate() const { return hUpdate; }
  //! Modify the H update rule.
  HUpdateRule& HUpdate() { return hUpdate; }

}; // class ALs

}; // namespace nmf
}; // namespace mlpack

// Include implementation.
#include "als_impl.hpp"

#endif
