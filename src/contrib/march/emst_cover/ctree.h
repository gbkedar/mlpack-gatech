/**
 * @file ctree.h
 * 
 * This file defines and prototypes functions of 
 * a namespace which makes a cover tree.
 *
 */

#ifndef TREE_COVER_TREE_H
#define TREE_COVER_TREE_H

#include <fastlib/fastlib.h>
#include "cover_tree.h"

#ifdef PARTIAL_DISTANCES
#include "distances.h"
#endif

const fx_entry_doc tree_construction_entries[] = {
  {"fc", FX_PARAM, FX_BOOL, NULL,
   " The parameter that decides whether we chose the child"
   " node randomly or the farthest point as the child.\n"},
  FX_ENTRY_DOC_DONE
};

const fx_module_doc tree_construction_doc = {
  tree_construction_entries, NULL,
  " Builds the cover tree as per the original"
  " algorithm.\n"
};

namespace ctree {

  // The module for the namespace
  datanode *module;

  // This is the expansion constant, default value 1.3
  // We assign its value when we start making the tree
  double EC = 1.3;

  // This returns the templated value of the 
  // expansion constant
  template<typename T> 
  inline T BASE() {
    T base = (T) EC;
    return base;
  }
  
  // This function computes the 1/log(base) 
  // required to compute the log of 
  // something with respect of this base
  template<typename T> 
  inline T inverse_log_base() {
    T inverse_log_base  = 1.0 / log(BASE<T>());
    return inverse_log_base;
  }

  // This is the lower bound on the lowest scale we 
  // can go to. This means distance between two points 
  // is zero
  const size_t NEG_INF = (int) log(0);

  // This returns the distance with respect to this 
  //scale
  template<typename T>
  inline T scaled_distance(size_t scale) {
    return pow(BASE<T>(), scale);
  }

  // This returns the scale with respect to a distance. 
  // Using this we can compute the scale of a particular 
  // node
  template<typename T>
  inline size_t scale_of_distance(T distance) {
    return (size_t) ceil(log(distance)*inverse_log_base<T>());
  }

  // This class stores a point and its distance to all 
  // the nodes which have taken this point as its 
  // potential descendant in a stack 
  template<typename T>
  class NodeDistances {
    
  private:
    // the point (index in the data matrix)
    size_t point_;
    // The array which stores the distances to all the 
    // nodes which have taken this point as a 
    // potential descendant in a stack
    ArrayList<T> distances_;
    
  public:
    
    NodeDistances() {
      distances_.Init(0);
    }
    
    ~NodeDistances() {
    }

    size_t point() {
      return point_;
    }

    ArrayList<T> *distances() {
      return &distances_;
    }

    T distances(size_t in) {
      return distances_[in];
    }

    void add_distance(T dist) {
      distances_.PushBackCopy(dist);
      return;
    }

    void Init(size_t point, T dist) {
      point_ = point;
      distances_.PushBackCopy(dist);
      return;
    }
  };

  // This function return the maximum of the 
  // last distances for a set of NodeDistances  
  // This is used to compute the maximum distance 
  // to any descendant and also to decide the next 
  // scale in the explicit representation
  template<typename T>
  T max_set(ArrayList<NodeDistances<T>*>*, size_t*);

  template<typename T>
  T max_set(ArrayList<NodeDistances<T>*> *set) {
    return max_set(set, NULL);
  }


  // used for printing purposes
  void print_space(size_t);

  // This function traverses down the tree in a depth first 
  // fashion, printing the nodes
  template<typename TCoverTreeNode>
  void print_tree(size_t, TCoverTreeNode*);

  /**
   * This public function prints the sub-tree 
   * under the node you provide it. If you 
   * provide the root then it prints the whole tree.
   *
   * Usage:
   * @code
   * ctree::PrintTree<CoverTreeType>(tree_node);
   * @endcode
   */
  template<typename TCoverTreeNode>
  void PrintTree(TCoverTreeNode *top_node) {
    size_t depth = 0;
    print_tree(depth, top_node);
    return;
  }

  // This function splits a set of NodeDistances
  // into the set of points which can be 
  // possible descendants of the self-child
  // of the node we are at
  // and points which would be possible 
  // descendants of the other children
  template<typename T>
  void split_far(ArrayList<NodeDistances<T>*>*, 
		 ArrayList<NodeDistances<T>*>*,
		 size_t);

  // This function splits a set of NodeDistances
  // into set of points which can be possible 
  // descendants of the child of the node we are at
  // and points which wouldn't be possible descendants
  // of that child
  template<typename T>
  void split_near(size_t, const GenMatrix<T>&,
		  ArrayList<NodeDistances<T>*>*,
		  ArrayList<NodeDistances<T>*>*,
		  size_t);

  // This function makes the tree given a particular point.
  // It makes a node out of the point and also forms the 
  // self child and the other child in a depth first 
  // fashion. The points which are not yet consumed 
  // are put in a set and the one consumed put in another
  template<typename TCoverTreeNode, typename T>
  TCoverTreeNode *private_make_tree(size_t, const GenMatrix<T>&,
				    size_t, size_t, 
				    ArrayList<NodeDistances<T>*>*,
				    ArrayList<NodeDistances<T>*>*);


  /**
   * This public function is used to make a cover 
   * tree on a particular dataset for a particular 
   * expansion constant
   *
   * Usage:
   * @code
   * GenMatrix<T> dataset;
   * T base = 2.0;
   * TreeType *tree = ctree::MakeCoverTree<TreeType, T>(dataset, base);
   * @endcode
   */
  template<typename TCoverTreeNode, typename T>
  TCoverTreeNode *MakeCoverTree(const GenMatrix<T>& dataset, T base,
				datanode *mod) {

    // setting the expansion constant here   
    module = mod; 
    EC = (double)base;
    size_t n = dataset.n_cols();
    DEBUG_ASSERT(n > 0);
    ArrayList<NodeDistances<T>*> point_set, consumed_set;
    GenVector<T> root_point;

    // choosing the first point in the dataset 
    // as the root
    dataset.MakeColumnVector(0, &root_point);
    point_set.Init(0);
    consumed_set.Init(0);

    // here we create the set of NodeDistances which 
    // would be used throughout the making of the tree.
    for (size_t i = 1; i < n; i++) {
      NodeDistances<T> *node_distances = new NodeDistances<T>();
      GenVector<T> point;
      T dist;

      dataset.MakeColumnVector(i, &point);
      
#ifdef PARTIAL_DISTANCES
      dist = pdc::DistanceEuclidean<T>(root_point, point, sqrt(DBL_MAX));
#else
      dist = sqrt(la::DistanceSqEuclidean(root_point, point));
#endif
      
      node_distances->Init(i, dist);
      *((point_set.PushBackRaw())) = node_distances;
    }
    DEBUG_ASSERT(point_set.size() == n - 1);

    // Setting the maximum scale of the explicit 
    // representation
    T max_dist = max_set(&point_set);
    size_t max_scale = scale_of_distance(max_dist);
    
    
    TCoverTreeNode *root_node = 
      private_make_tree<TCoverTreeNode, T>(0, dataset, max_scale,
					   max_scale, &point_set,
					   &consumed_set);

    for(size_t i = 0; i < point_set.size(); i++) {
      delete point_set[i];
    }
    for(size_t i = 0; i < consumed_set.size(); i++) {
      delete consumed_set[i];
    }

    return root_node;
  }

};

#include "ctree_impl.h"
#endif
