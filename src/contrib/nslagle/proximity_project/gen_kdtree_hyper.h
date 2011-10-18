// Copyright 2007 Georgia Institute of Technology. All rights reserved.
// ABSOLUTELY NOT FOR DISTRIBUTCLIN
/**
 * @file tree/kdtree.h
 *
 * Tools for kd-trees.
 *
 * Eventually we hope to support KD trees with non-L2 (Euclidean)
 * metrics, like Manhattan distance.
 *
 * @experimental
 */

#ifndef TREE_GEN_KDTREE_HYPER_H
#define TREE_GEN_KDTREE_HYPER_H

#include "general_spacetree.h"
#include "general_type_bounds.h"

#include "gen_kdtree_hyper_impl.h"

/**
 * Regular pointer-style trees (as opposed to THOR trees).
 */
namespace proximity {

  /**
   * Creates a KD tree from hyperrectangles
   *
   * @experimental
   *
   * This requires you to pass in two unitialized ArrayLists which will contain
   * index mappings so you can account for the re-ordering of the matrix.
   * (By unitialized I mean don't call Init on it)
   *
   * @param matrix data where each column is a point, WHICH WILL BE RE-ORDERED
   * @param leaf_size the maximum points in a leaf
   * @param old_from_new pointer to an unitialized arraylist; it will map
   *        new indices to original
   * @param new_from_old pointer to an unitialized arraylist; it will map
   *        original indexes to new indices
   */
  template<typename T, typename TKdTree, typename TKdTreeSplitter>
  TKdTree *MakeGenKdTree(GenMatrix<T>& lower_limit_matrix,
			 GenMatrix<T>& upper_limit_matrix,
			 size_t leaf_size,
			 ArrayList<size_t> *old_from_new = NULL,
			 ArrayList<size_t> *new_from_old = NULL) {

    TKdTree *node = new TKdTree();
    size_t *old_from_new_ptr;

    if (old_from_new) {
      old_from_new->Init(lower_limit_matrix.n_cols());

      for (size_t i = 0; i < lower_limit_matrix.n_cols(); i++) {
        (*old_from_new)[i] = i;
      }

      old_from_new_ptr = old_from_new->begin();
    }
    else {
      old_from_new_ptr = NULL;
    }

    node->Init(0, lower_limit_matrix.n_cols());
    node->bound().Init(lower_limit_matrix.n_rows());
    tree_gen_kdtree_private::FindBoundFromMatrix
      (lower_limit_matrix, upper_limit_matrix, 0, lower_limit_matrix.n_cols(),
       &node->bound());

    tree_gen_kdtree_private::SplitGenKdTree<T, TKdTree, TKdTreeSplitter>
      (lower_limit_matrix, upper_limit_matrix, node, leaf_size,
       old_from_new_ptr);

    if (new_from_old) {
      new_from_old->Init(lower_limit_matrix.n_cols());
      for (size_t i = 0; i < lower_limit_matrix.n_cols(); i++) {
        (*new_from_old)[(*old_from_new)[i]] = i;
      }
    }

    return node;
  }

};

#endif
