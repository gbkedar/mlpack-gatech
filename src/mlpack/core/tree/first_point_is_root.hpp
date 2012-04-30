/**
 * @file first_point_is_root.hpp
 * @author Ryan Curtin
 *
 * A very simple policy for the cover tree; the first point in the dataset is
 * chosen as the root of the cover tree.
 */
#ifndef __MLPACK_CORE_TREE_FIRST_POINT_IS_ROOT_HPP
#define __MLPACK_CORE_TREE_FIRST_POINT_IS_ROOT_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace tree {

/**
 * This class is meant to be used as a choice for the policy class
 * RootPointPolicy of the CoverTree class.  This policy determines which point
 * is used for the root node of the cover tree.  This particular implementation
 * simply chooses the first point in the dataset as the root.  A more complex
 * implementation might choose, for instance, the point with least maximum
 * distance to other points (the closest to the "middle").
 */
class FirstPointIsRoot
{
 public:
  /**
   * Return the point to be used as the root point of the cover tree.  This just
   * returns 0.
   */
  static size_t ChooseRoot(const arma::mat& /* dataset */) { return 0; }
};

}; // namespace tree
}; // namespace mlpack

#endif // __MLPACK_CORE_TREE_FIRST_POINT_IS_ROOT_HPP
