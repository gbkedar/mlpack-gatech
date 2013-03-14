/**
 * @file fastmks_rules.hpp
 * @author Ryan Curtin
 *
 * Rules for the single or dual tree traversal for fast max-kernel search.
 */
#ifndef __MLPACK_METHODS_FASTMKS_FASTMKS_RULES_HPP
#define __MLPACK_METHODS_FASTMKS_FASTMKS_RULES_HPP

#include <mlpack/core.hpp>
#include <mlpack/core/tree/cover_tree/cover_tree.hpp>

namespace mlpack {
namespace fastmks {

template<typename KernelType, typename TreeType>
class FastMKSRules
{
 public:
  FastMKSRules(const arma::mat& referenceSet,
               const arma::mat& querySet,
               arma::Mat<size_t>& indices,
               arma::mat& products,
               KernelType& kernel);

  //! Compute the base case (kernel value) between two points.
  double BaseCase(const size_t queryIndex, const size_t referenceIndex);

  /**
   * Get the score for recursion order.  A low score indicates priority for
   * recursion, while DBL_MAX indicates that the node should not be recursed
   * into at all (it should be pruned).
   *
   * @param queryIndex Index of query point.
   * @param referenceNode Candidate to be recursed into.
   */
  double Score(const size_t queryIndex, TreeType& referenceNode) const;

  /**
   * Get the score for recursion order, passing the base case result (in the
   * situation where it may be needed to calculate the recursion order).  A low
   * score indicates priority for recursion, while DBL_MAX indicates that the
   * node should not be recursed into at all (it should be pruned).
   *
   * @param queryIndex Index of query point.
   * @param referenceNode Candidate node to be recursed into.
   * @param baseCaseResult Result of BaseCase(queryIndex, referenceNode).
   */
  double Score(const size_t queryIndex,
               TreeType& referenceNode,
               const double baseCaseResult) const;

  /**
   * Get the score for recursion order.  A low score indicates priority for
   * recursion, while DBL_MAX indicates that the node should not be recursed
   * into at all (it should be pruned).
   *
   * @param queryNode Candidate query node to be recursed into.
   * @param referenceNode Candidate reference node to be recursed into.
   */
  double Score(TreeType& queryNode, TreeType& referenceNode) const;

  /**
   * Get the score for recursion order, passing the base case result (in the
   * situation where it may be needed to calculate the recursion order).  A low
   * score indicates priority for recursion, while DBL_MAX indicates that the
   * node should not be recursed into at all (it should be pruned).
   *
   * @param queryNode Candidate query node to be recursed into.
   * @param referenceNode Candidate reference node to be recursed into.
   * @param baseCaseResult Result of BaseCase(queryNode, referenceNode).
   */
  double Score(TreeType& queryNode,
               TreeType& referenceNode,
               const double baseCaseResult) const;

  /**
   * Re-evaluate the score for recursion order.  A low score indicates priority
   * for recursion, while DBL_MAX indicates that a node should not be recursed
   * into at all (it should be pruned).  This is used when the score has already
   * been calculated, but another recursion may have modified the bounds for
   * pruning.  So the old score is checked against the new pruning bound.
   *
   * @param queryIndex Index of query point.
   * @param referenceNode Candidate node to be recursed into.
   * @param oldScore Old score produced by Score() (or Rescore()).
   */
  double Rescore(const size_t queryIndex,
                 TreeType& referenceNode,
                 const double oldScore) const;

  /**
   * Re-evaluate the score for recursion order.  A low score indicates priority
   * for recursion, while DBL_MAX indicates that a node should not be recursed
   * into at all (it should be pruned).  This is used when the score has already
   * been calculated, but another recursion may have modified the bounds for
   * pruning.  So the old score is checked against the new pruning bound.
   *
   * @param queryNode Candidate query node to be recursed into.
   * @param referenceNode Candidate reference node to be recursed into.
   * @param oldScore Old score produced by Score() (or Rescore()).
   */
  double Rescore(TreeType& queryNode,
                 TreeType& referenceNode,
                 const double oldScore) const;

 private:
  const arma::mat& referenceSet;

  const arma::mat& querySet;

  arma::Mat<size_t>& indices;

  arma::mat& products;

  arma::vec queryKernels; // || q || for each q.
  arma::vec referenceKernels;

  //! The instantiated kernel.
  KernelType& kernel;

  //! Calculate the bound for a given query node.
  double CalculateBound(TreeType& queryNode) const;

  void InsertNeighbor(const size_t queryIndex,
                      const size_t pos,
                      const size_t neighbor,
                      const double distance);
};

}; // namespace fastmks
}; // namespace mlpack

// Include implementation.
#include "fastmks_rules_impl.hpp"

#endif
