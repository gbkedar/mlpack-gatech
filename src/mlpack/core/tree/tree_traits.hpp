/**
 * @file tree_traits.hpp
 * @author Ryan Curtin
 *
 * This file implements the basic, unspecialized TreeTraits class, which
 * provides information about tree types.  If you create a tree class, you
 * should specialize this class with the characteristics of your tree.
 */
#ifndef __MLPACK_CORE_TREE_TREE_TRAITS_HPP
#define __MLPACK_CORE_TREE_TREE_TRAITS_HPP

namespace mlpack {
namespace tree {

/**
 * The TreeTraits class provides compile-time information on the characteristics
 * of a given tree type.  These include traits such as whether or not a node
 * knows the distance to its parent node, or whether or not the subspaces
 * represented by children can overlap.
 *
 * These traits can be used for static compile-time optimization:
 *
 * @code
 * // This if statement will be optimized out at compile time!
 * if (TreeTraits<TreeType>::HasOverlappingChildren == false)
 * {
 *   // Do a simpler computation because no children overlap.
 * }
 * else
 * {
 *   // Do the full, complex calculation.
 * }
 * @endcode
 *
 * The traits can also be used in conjunction with SFINAE to write specialized
 * versions of functions:
 *
 * @code
 * template<typename TreeType>
 * void Compute(TreeType& node,
 *              boost::enable_if<
 *                  TreeTraits<TreeType>::HasParentDistance>::type*)
 * {
 *   // Computation with TreeType::ParentDistance().
 * }
 *
 * template<typename TreeType>
 * void Compute(TreeType& node,
 *              boost::enable_if<
 *                  !TreeTraits<TreeType>::HasParentDistance>::type*)
 * {
 *   // Computation without TreeType::ParentDistance().
 * }
 * @endcode
 *
 * In those two examples, the boost::enable_if<> class takes a boolean template
 * parameter which allows that function to be called when the boolean is true.
 *
 * Each trait must be a static const value and not a function; only const values
 * can be used as template parameters (with the exception of constexprs, which
 * are a C++11 feature; but MLPACK is not using C++11).  By default (the
 * unspecialized implementation of TreeTraits), each parameter is set to make as
 * few assumptions about the tree as possible; so, even if TreeTraits is not
 * specialized for a particular tree type, tree-based algorithms should still
 * work.
 *
 * When you write your own tree, you must specialize the TreeTraits class to
 * your tree type and set the corresponding values appropriately.  See
 * mlpack/core/tree/binary_space_tree/traits.hpp for an example.
 */
template<typename TreeType>
class TreeTraits
{
 public:
  /**
   * This is true if TreeType::ParentDistance() exists and works.  The
   * ParentDistance() function returns the distance between the center of a node
   * and the center of its parent.
   */
  static const bool HasParentDistance = false;

  /**
   * This is true if the subspaces represented by the children of a node can
   * overlap.
   */
  static const bool HasOverlappingChildren = true;

  /**
   * This is true if Point(0) is the centroid of the node.
   */
  static const bool FirstPointIsCentroid = false;
};

}; // namespace tree
}; // namespace mlpack

#endif
