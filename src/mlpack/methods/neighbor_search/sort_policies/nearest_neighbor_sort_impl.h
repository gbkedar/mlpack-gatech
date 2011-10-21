/***
 * @file nearest_neighbor_sort_impl.h
 * @author Ryan Curtin
 *
 * Implementation of templated methods for the NearestNeighborSort SortPolicy
 * class for the NeighborSearch class.
 */
#ifndef __MLPACK_NEIGHBOR_NEAREST_NEIGHBOR_SORT_IMPL_H
#define __MLPACK_NEIGHBOR_NEAREST_NEIGHBOR_SORT_IMPL_H

#include <mlpack/core/kernels/lmetric.hpp>

namespace mlpack {
namespace neighbor {

template<typename TreeType>
double NearestNeighborSort::BestNodeToNodeDistance(TreeType* query_node,
                                                   TreeType* reference_node) {
  // This is not implemented yet for the general case because the trees do not
  // accept arbitrary distance metrics.
  return query_node->bound().MinDistance(reference_node->bound());
}

template<typename TreeType>
double NearestNeighborSort::BestPointToNodeDistance(const arma::vec& point,
                                                    TreeType* reference_node) {
  // This is not implemented yet for the general case because the trees do not
  // accept arbitrary distance metrics.
  return reference_node->bound().MinDistance(point);
}

}; // namespace neighbor
}; // namespace mlpack

#endif
