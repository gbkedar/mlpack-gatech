/**
 * @file unmap.hpp
 * @author Ryan Curtin
 *
 * Convenience methods to unmap results.
 */
#ifndef __MLPACK_METHODS_NEIGHBOR_SEARCH_UNMAP_HPP
#define __MLPACK_METHODS_NEIGHBOR_SEARCH_UNMAP_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace neighbor {

/**
 * Assuming that the datasets have been mapped using the referenceMap and the
 * queryMap (such as during kd-tree construction), unmap the columns of the
 * distances and neighbors matrices into neighborsOut and distancesOut, and also
 * unmap the entries in each row of neighbors.  This is useful for the dual-tree
 * case.
 *
 * @param neighbors Matrix of neighbors resulting from neighbor search.
 * @param distances Matrix of distances resulting from neighbor search.
 * @param referenceMap Mapping of reference set to old points.
 * @param queryMap Mapping of query set to old points.
 * @param neighborsOut Matrix to store unmapped neighbors into.
 * @param distancesOut Matrix to store unmapped distances into.
 * @param squareRoot If true, take the square root of the distances.
 */
void Unmap(const arma::Mat<size_t>& neighbors,
           const arma::mat& distances,
           const std::vector<size_t>& referenceMap,
           const std::vector<size_t>& queryMap,
           arma::Mat<size_t>& neighborsOut,
           arma::mat& distancesOut,
           const bool squareRoot = false);

/**
 * Assuming that the datasets have been mapped using referenceMap (such as
 * during kd-tree construction), unmap the columns of the distances and
 * neighbors matrices into neighborsOut and distancesOut, and also unmap the
 * entries in each row of neighbors.  This is useful for the single-tree case.
 *
 * @param neighbors Matrix of neighbors resulting from neighbor search.
 * @param distances Matrix of distances resulting from neighbor search.
 * @param referenceMap Mapping of reference set to old points.
 * @param neighborsOut Matrix to store unmapped neighbors into.
 * @param distancesOut Matrix to store unmapped distances into.
 * @param squareRoot If true, take the square root of the distances.
 */
void Unmap(const arma::Mat<size_t>& neighbors,
           const arma::mat& distances,
           const std::vector<size_t>& referenceMap,
           arma::Mat<size_t>& neighborsOut,
           arma::mat& distancesOut,
           const bool squareRoot = false);

}; // namespace neighbor
}; // namespace mlpack

#endif
