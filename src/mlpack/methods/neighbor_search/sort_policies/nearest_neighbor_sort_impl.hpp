/**
 * @file nearest_neighbor_sort_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of templated methods for the NearestNeighborSort SortPolicy
 * class for the NeighborSearch class.
 */
#ifndef __MLPACK_NEIGHBOR_NEAREST_NEIGHBOR_SORT_IMPL_HPP
#define __MLPACK_NEIGHBOR_NEAREST_NEIGHBOR_SORT_IMPL_HPP

namespace mlpack {
namespace neighbor {

template<typename TreeType>
inline double NearestNeighborSort::BestNodeToNodeDistance(
    const TreeType* queryNode,
    const TreeType* referenceNode)
{
  // This is not implemented yet for the general case because the trees do not
  // accept arbitrary distance metrics.
  return queryNode->MinDistance(referenceNode);
}

template<typename TreeType>
inline double NearestNeighborSort::BestNodeToNodeDistance(
    const TreeType* queryNode,
    const TreeType* referenceNode,
    const double centerToCenterDistance)
{
  return queryNode->MinDistance(referenceNode, centerToCenterDistance);
}

template<typename TreeType>
inline double NearestNeighborSort::BestPointToNodeDistance(
    const arma::vec& point,
    const TreeType* referenceNode)
{
  // This is not implemented yet for the general case because the trees do not
  // accept arbitrary distance metrics.
  return referenceNode->MinDistance(point);
}

template<typename TreeType>
inline double NearestNeighborSort::BestPointToNodeDistance(
    const arma::vec& point,
    const TreeType* referenceNode,
    const double pointToCenterDistance)
{
  return referenceNode->MinDistance(point, pointToCenterDistance);
}

}; // namespace neighbor
}; // namespace mlpack

#endif
