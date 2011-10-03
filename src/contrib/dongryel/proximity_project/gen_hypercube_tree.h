#ifndef GEN_HYPERCUBE_TREE_H
#define GEN_HYPERCUBE_TREE_H

#include "fastlib/fastlib.h"
#include "fastlib/tree/statistic.h"

#include "gen_hypercube_tree_impl.h"

namespace proximity {

  template<class TStatistic>
  class GenHypercubeTree {

   public:

    typedef DHrectBound<2> Bound;
    typedef Matrix Dataset;
    typedef TStatistic Statistic;
    
    Bound bound_;
    ArrayList<GenHypercubeTree *> children_;
    ArrayList<size_t> begin_;
    ArrayList<size_t> count_;
    size_t total_count_;
    size_t level_;
    unsigned int node_index_;
    Statistic stat_;
    
    
   public:

    GenHypercubeTree() {
    }

    ~GenHypercubeTree() {
      if(children_.size() > 0) {
	for(size_t i = 0; i < children_.size(); i++) {
	  delete children_[i];
	}
      }      
    }

    const Statistic& stat() const {
      return stat_;
    }
    
    Statistic& stat() {
      return stat_;
    }

    /** @brief Tests whether the current node is a leaf node
     *         (childless).
     *
     *  @return true if childless, false otherwise.
     */
    bool is_leaf() const {
      return children_.size() == 0;
    }

    void Init(size_t number_of_particle_sets, size_t dimension) {
      begin_.Init(number_of_particle_sets);
      count_.Init(number_of_particle_sets);
      total_count_ = 0;
      node_index_ = 0;
      children_.Init();
    }

    void Init(size_t particle_set_number, size_t begin_in, 
	      size_t count_in) {

      begin_[particle_set_number] = begin_in;
      count_[particle_set_number] = count_in;
      total_count_ += count_in;
    }

    double side_length() const {
      const DRange &range = bound_.get(0);
      return range.hi - range.lo;
    }

    const Bound& bound() const {
      return bound_;
    }
    
    Bound& bound() {
      return bound_;
    }

    GenHypercubeTree *get_child(int index) const {
      return children_[index];
    }

    void set_level(size_t level) {
      level_ = level;
    }

    GenHypercubeTree *AllocateNewChild(size_t number_of_particle_sets,
				       size_t dimension,
				       unsigned int node_index_in) {
      
      GenHypercubeTree *new_node = new GenHypercubeTree();
      *(children_.PushBackRaw()) = new_node;

      new_node->Init(number_of_particle_sets, dimension);
      new_node->node_index_ = node_index_in;

      return new_node;
    }

    /**
     * Gets the index of the begin point of this subset.
     */
    size_t begin(size_t particle_set_number) const {
      return begin_[particle_set_number];
    }
    
    /**
     * Gets the index one beyond the last index in the series.
     */
    size_t end(size_t particle_set_number) const {
      return begin_[particle_set_number] + count_[particle_set_number];
    }
    
    unsigned int node_index() const {
      return node_index_;
    }

    /**
     * Gets the number of points in this subset.
     */
    size_t count(size_t particle_set_number) const {
      return count_[particle_set_number];
    }

    size_t count() const {
      return total_count_;
    }

    size_t level() const {
      return level_;
    }

    /**
     * Gets the number of children.
     */
    size_t num_children() const {
      return children_.size();
    }

    void Print() const {
      if (!is_leaf()) {
	printf("internal node: %d points total on level %d\n", total_count_,
	       level_);
	printf("  bound:\n");
	for(size_t i = 0; i < bound_.dim(); i++) {
	  printf("%g %g\n", bound_.get(i).lo, bound_.get(i).hi);
	}
	for(size_t i = 0; i < begin_.size(); i++) {
	  printf("   set %d: %d to %d: %d points total\n", i, 
		 begin_[i], begin_[i] + count_[i] - 1, count_[i]);	  
	}
	for(size_t c = 0; c < children_.size(); c++) {
	  children_[c]->Print();
	}
      }
      else {
	printf("leaf node: %d points total on level %d\n", total_count_,
	       level_);
	printf("  bound:\n");
	for(size_t i = 0; i < bound_.dim(); i++) {
	  printf("%g %g\n", bound_.get(i).lo, bound_.get(i).hi);
	}
	for(size_t i = 0; i < begin_.size(); i++) {
	  printf("   set %d: %d to %d: %d points total\n", i, 
		 begin_[i], begin_[i] + count_[i] - 1, count_[i]);	  
	}
      }
    }

  };


  /** @brief Creates a generalized hypercube tree (high-dimensional
   * generalization of quad-tree, octree) from data.
   *
   * @experimental
   *
   * This requires you to pass in two unitialized ArrayLists which
   * will contain index mappings so you can account for the
   * re-ordering of the matrix.  (By unitialized I mean don't call
   * Init on it)
   *
   * @param matrix data where each column is a point, WHICH WILL BE
   * RE-ORDERED
   *
   * @param leaf_size the maximum points in a leaf
   *
   * @param old_from_new pointer to an unitialized arraylist; it
   * will map new indices to original
   *
   * @param new_from_old pointer to an unitialized arraylist; it
   * will map original indexes to new indices
   */
  template<typename TStatistic>
  GenHypercubeTree<TStatistic> *MakeGenHypercubeTree
  (ArrayList<Matrix *> &matrices, size_t leaf_size, size_t max_tree_depth,
   ArrayList< ArrayList<GenHypercubeTree<TStatistic> *> > *nodes_in_each_level,
   ArrayList< ArrayList<size_t> > *old_from_new = NULL,
   ArrayList< ArrayList<size_t> > *new_from_old = NULL) {
    
    GenHypercubeTree<TStatistic> *node = new GenHypercubeTree<TStatistic>();
    
    if (old_from_new) {
      old_from_new->Init(matrices.size());

      for(size_t j = 0; j < matrices.size(); j++) {
	(*old_from_new)[j].Init(matrices[j]->n_cols());
	
	for (size_t i = 0; i < matrices[j]->n_cols(); i++) {
	  (*old_from_new)[j][i] = i;
	}
      }
    }
    
    // Initialize the global list of nodes.
    nodes_in_each_level->Init(max_tree_depth + 1);
    for(size_t i = 0; i < nodes_in_each_level->size(); i++) {
      ((*nodes_in_each_level)[i]).Init();
    }

    // Initialize the root node.
    node->Init(matrices.size(), matrices[0]->n_rows());
    node->set_level(0);
    for(size_t i = 0; i < matrices.size(); i++) {
      node->Init(i, 0, matrices[i]->n_cols());
    }
    
    // Make the tightest cube bounding box you can fit around the
    // current set of points.
    tree_gen_hypercube_tree_private::ComputeBoundingHypercube(matrices, node);

    // Put the root node into the initial list of level 0.
    *(((*nodes_in_each_level)[0]).PushBackRaw()) = node;

    tree_gen_hypercube_tree_private::SplitGenHypercubeTree
      (matrices, node, leaf_size, max_tree_depth, nodes_in_each_level, 
       old_from_new, 0);

    // Index shuffling business...
    if (new_from_old) {
      new_from_old->Init(matrices.size());

      for(size_t j = 0; j < matrices.size(); j++) {
	(*new_from_old)[j].Init(matrices[j]->n_cols());
	for (size_t i = 0; i < matrices[j]->n_cols(); i++) {
	  (*new_from_old)[j][(*old_from_new)[j][i]] = i;
	}
      }
    }
    
    return node;
  }
};

#endif
