/***
 * @file furthest_neighbor_sort.h
 * @author Ryan Curtin
 *
 * Implementation of the SortPolicy class for NeighborSearch; in this case, the
 * furthest neighbors are those that are most important.
 */
#ifndef __MLPACK_NEIGHBOR_FURTHEST_NEIGHBOR_SORT_H
#define __MLPACK_NEIGHBOR_FURTHEST_NEIGHBOR_SORT_H

#include <mlpack/core.h>

namespace mlpack {
namespace neighbor {

/***
 * This class implements the necessary methods for the SortPolicy template
 * parameter of the NeighborSearch class.  The sorting policy here is that the
 * minimum distance is the best (so, the output is nearest neighbors).
 */
class FurthestNeighborSort {
 public:
  /***
   * Return the index in the vector where the new distance should be inserted,
   * or size_t() - 1 if it should not be inserted (i.e. if it is not any better
   * than any of the existing points in the list).  The list should be sorted
   * such that the best point is the first in the list.  The actual insertion is
   * not performed.
   *
   * @param list Vector of existing distance points
   * @param new_distance Distance to try to insert
   *
   * @return size_t containing the position to insert into, or (size_t() - 1)
   *     if the new distance should not be inserted.
   */
  static size_t SortDistance(arma::vec& list, double new_distance);

  /***
   * Return whether or not value is "better" than ref.  In this case, that means
   * that the value is greater than the reference.
   *
   * @param value Value to compare
   * @param ref Value to compare with
   *
   * @return bool indicating whether or not (value > ref).
   */
  static inline bool IsBetter(const double value, const double ref) {
    return (value > ref);
  }

  /***
   * Return the best possible distance between two nodes.  In our case, this is
   * the maximum distance between the two tree nodes using the given distance
   * function.
   */
  template<typename TreeType>
  static double BestNodeToNodeDistance(TreeType* query_node,
                                       TreeType* reference_node);

  /***
   * Return the best possible distance between a node and a point.  In our case,
   * this is the maximum distance between the tree node and the point using the
   * given distance function.
   */
  template<typename TreeType>
  static double BestPointToNodeDistance(const arma::vec& query_point,
                                        TreeType* reference_node);

  /***
   * Return what should represent the worst possible distance with this
   * particular sort policy.  In our case, this should be the minimum possible
   * distance, 0.
   *
   * @return 0
   */
  static inline const double WorstDistance() { return 0; }

  /***
   * Return what should represent the best possible distance with this
   * particular sort policy.  In our case, this should be the maximum possible
   * distance, DBL_MAX.
   *
   * @return DBL_MAX
   */
  static inline const double BestDistance() { return DBL_MAX; }
};

}; // namespace neighbor
}; // namespace mlpack

// Include implementation of templated functions.
#include "furthest_neighbor_sort_impl.h"

#endif
