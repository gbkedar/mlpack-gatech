/**
 * @file approx_nn.h
 *
 * Defines ApproxNN class to perform all-nearest-neighbors on two specified 
 * data sets, but obtain the approximate rank nearest neighbor with a 
 * given probability.
 */

#ifndef APPROX_NN_H
#define APPROX_NN_H

#include <fastlib/fastlib.h>
#include <vector>

const fx_entry_doc approx_nn_entries[] = {
  {"dim", FX_PARAM, FX_INT, NULL,
   " The dimension of the data we are dealing with.\n"},
  {"qsize", FX_PARAM, FX_INT, NULL,
   " The number of points in the query set.\n"},
  {"rsize", FX_PARAM, FX_INT, NULL, 
   " The number of points in the reference set.\n"},
  {"knns", FX_PARAM, FX_INT, NULL, 
   " The number of nearest neighbors we need to compute"
   " (defaults to 1).\n"},
//   {"epsilon", FX_PARAM, FX_INT, NULL,
//    " Rank approximation.\n"},
  {"epsilon", FX_PARAM, FX_DOUBLE, NULL,
   " Rank approximation factor (\% of the reference set size).\n"},
  {"alpha", FX_PARAM, FX_DOUBLE, NULL,
   " The error probability.\n"},
  {"leaf_size", FX_PARAM, FX_INT, NULL,
   " The leaf size for the kd-tree.\n"},
  {"sample_limit", FX_PARAM, FX_INT, NULL,
   " The maximum number of samples"
   " allowed to be made from a single node.\n"},
  {"naive_init", FX_TIMER, FX_CUSTOM, NULL,
   "Naive initialization time.\n"},
  {"naive", FX_TIMER, FX_CUSTOM, NULL,
   "Naive computation time.\n"},
  {"exact_init", FX_TIMER, FX_CUSTOM, NULL,
   "Exact initialization time.\n"},
  {"exact", FX_TIMER, FX_CUSTOM, NULL,
   "Exact computation time.\n"},
  {"approx_init", FX_TIMER, FX_CUSTOM, NULL,
   "Approx initialization time.\n"},
  {"approx", FX_TIMER, FX_CUSTOM, NULL,
   "Approximate computation time.\n"},
  {"tree_building", FX_TIMER, FX_CUSTOM, NULL,
   " The timer to record the time taken to build" 
   " the query and the reference tree.\n"},
  {"tree_building_approx", FX_TIMER, FX_CUSTOM, NULL,
   " The timer to record the time taken to build" 
   " the query and the reference tree for InitApprox.\n"},
  {"computing_sample_sizes", FX_TIMER, FX_CUSTOM, NULL,
   " The timer to compute the sample sizes.\n"},
  FX_ENTRY_DOC_DONE
};

const fx_module_doc approx_nn_doc = {
  approx_nn_entries, NULL,
  " Performs approximate nearest neighbors computation"
  " - exact, approximate, brute.\n"
};


size_t dc = 0;
size_t mc = 0;



/**
 * Performs all-nearest-neighbors.  This class will build the trees and 
 * perform the recursive  computation.
 */
class ApproxNN {
  
  //////////////////////////// Nested Classes /////////////////////////
  /**
   * Extra data for each node in the tree.  For all nearest neighbors, 
   * each node only
   * needs its upper bound on its nearest neighbor distances.  
   */
  class QueryStat {
    
    // Defines many useful things for a class, including a pretty 
    // printer and copy constructor
    OT_DEF_BASIC(QueryStat) {
      // Include this line for all non-pointer members
      // There are other versions for arrays and pointers, see base/otrav.h
      OT_MY_OBJECT(max_distance_so_far_); 
      OT_MY_OBJECT(total_points_);
      OT_MY_OBJECT(samples_);
    } // OT_DEF_BASIC
    
  private:
    // The upper bound on the node's nearest neighbor distances.
    double max_distance_so_far_;
    // Number of points considered
    size_t total_points_;
    // Number of points sampled
    size_t samples_;
    
  public:
    // getters
    double max_distance_so_far() {
      return max_distance_so_far_; 
    } 

    size_t total_points() {
      return total_points_;
    }

    size_t samples() {
      return samples_;
    }

    // setters
    void set_max_distance_so_far(double new_dist) {
      max_distance_so_far_ = new_dist; 
    } 

    void set_total_points(size_t points) {
      total_points_ = points;
    }

    void add_total_points(size_t points) {
      total_points_ += points;
    }

    void set_samples(size_t points) {
      samples_ = points;
    }

    void add_samples(size_t points) {
      samples_ += points;
    }
    
    // In addition to any member variables for the statistic, all stat 
    // classes need two Init 
    // functions, one for leaves and one for non-leaves. 
    
    /**
     * Initialization function used in tree-building when initializing 
     * a leaf node.  For allnn, needs no additional information 
     * at the time of tree building.  
     */
    void Init(const Matrix& matrix, size_t start, size_t count) {
      // The bound starts at infinity
      max_distance_so_far_ = DBL_MAX;
      // The points considered starts at zero
      total_points_ = 0;
      // The number of samples starts at zero
      samples_ = 0;
    } 
     
    /**
     * Initialization function used in tree-building when 
     * initializing a non-leaf node.  For other algorithms,
     * node statistics can be built using information from the children.  
     */
    void Init(const Matrix& matrix, size_t start, size_t count, 
	      const QueryStat& left, const QueryStat& right) {
      // For allnn, non-leaves can be initialized in the same way as leaves
      Init(matrix, start, count);
    } 
    
  }; //class QueryStat
  
  // TreeType are BinarySpaceTrees where the data are bounded by 
  // Euclidean bounding boxes, the data are stored in a Matrix, 
  // and each node has a QueryStat for its bound.
  typedef BinarySpaceTree<DHrectBound<2>, Matrix, QueryStat> TreeType;
   
  
  /////////////////////////////// Members ////////////////////////////
private:
  // These will store our data sets.
  Matrix queries_;
  Matrix references_;
  // This will store the query index for the single tree run
  size_t query_;
  // Pointers to the roots of the two trees.
  std::vector<TreeType*> query_trees_;
  TreeType* reference_tree_;
  // The total number of prunes.
  size_t number_of_prunes_;
  // A permutation of the indices for tree building.
  ArrayList<size_t> old_from_new_queries_;
  ArrayList<size_t> old_from_new_references_;
  // The number of points in a leaf
  size_t leaf_size_;
  // The distance to the candidate nearest neighbor for each query
  Vector neighbor_distances_;
  // The indices of the candidate nearest neighbor for each query
  ArrayList<size_t> neighbor_indices_;
  // number of nearest neighbrs
  size_t knns_; 
  // The module containing the parameters for this computation. 
  struct datanode* module_;
  // The array containing the sample sizes for the corresponding
  // set sizes
  ArrayList<size_t> sample_sizes_;
  // The rank approximation
  size_t rank_approx_;
  double epsilon_;
  // The maximum number of points to be sampled
  size_t sample_limit_;
  // Minimum number of samples required for each query
  // to maintain the probability bound for the error
  size_t min_samples_per_q_;  
  
  /////////////////////////////// Constructors ////////////////////////
  
  // Add this at the beginning of a class to prevent accidentally
  // calling the copy constructor
  FORBID_ACCIDENTAL_COPIES(ApproxNN);
  
public:
  /**
   * Constructors are generally very simple in FASTlib;
   * most of the work is done by Init().  This is only
   * responsible for ensuring that the object is ready
   * to be destroyed safely.  
   */
  ApproxNN() {
    reference_tree_ = NULL;
    query_trees_.clear();
  } 
  
  /**
   * The tree is the only member we are responsible for deleting.
   * The others will take care of themselves.  
   */
  ~ApproxNN() {
    for (std::vector<TreeType*>::iterator it = query_trees_.begin();
	 it < query_trees_.end(); it++) {
      if (*it != NULL) {
	delete *it;
      }
    }
    query_trees_.clear();
    if (reference_tree_ != NULL) {
      delete reference_tree_;
    }
  } 
    
  /////////////////////////// Helper Functions //////////////////////
  
private:
  /**
   * Computes the minimum squared distance between the
   * bounding boxes of two nodes
   */
  double MinNodeDistSq_ (TreeType* query_node, TreeType* reference_node) {
    mc++;
    return query_node->bound().MinDistanceSq(reference_node->bound());
  } 

  /**
   * This function computes the probability of
   * a particular quantile given the set and sample sizes
   * Computes P(d_(1) <= d_(1+rank_approx))
   */
  double ComputeProbability_(size_t set_size,
			     size_t sample_size,
			     size_t rank_approx) {
    double sum;
    Vector temp_a, temp_b;

    temp_a.Init(rank_approx+1);
    temp_a.SetAll(1.0);
    temp_b.Init(rank_approx+1);

    // calculating the temp_b
    temp_b[rank_approx] = 1;
    size_t i, j = 1;
    for (i = rank_approx-1; i > -1; i--, j++) {
      double frac = (double)(set_size-(sample_size-1)-j)
	/ (double)(set_size - j);
      temp_b[i] = temp_b[i+1]*frac;
    }
    DEBUG_ASSERT(j == rank_approx + 1);

    // computing the sum and the product with n/N
    sum = la::Dot(temp_a, temp_b);
    double prob = (double) sample_size / (double) set_size;
    prob *= sum;

//     // asserting that the probability is <= 1.0
// This may not be the case when 'n' is close to 'N' and 
// 'rank_approx' is large enough to make N-1-rank_approx+i < n-1
    return prob;
  }
  
  /**
   * This function computes the minimum sample sizes
   * required to obtain the approximate rank with
   * a given probability (1-alpha).
   * 
   * It assumes that the ArrayList<size_t> *samples
   * has been initialized to length N.
   */
  void ComputeSampleSizes_(size_t rank_approx, double alpha,
			   ArrayList<size_t> *samples) {
    size_t set_size = samples->size(),
      n = rank_approx + 1000;
    
    double prob;
    DEBUG_ASSERT(alpha <= 1.0);
    do {
      n--;
      prob = ComputeProbability_(rank_approx + 1000,
				 n, rank_approx);
    } while (prob >= alpha);

    double beta = (double) ++n
      / (double) (rank_approx + 1000);

    while (set_size > rank_approx) {
      (*samples)[set_size -1] = (size_t)(beta * (double) set_size);
      set_size--;
    }
    while (set_size > 0) {
      (*samples)[--set_size] = 1;
    }
  }

  /**
   * Performs exhaustive computation between two leaves.  
   */
  void ComputeBaseCase_(TreeType* query_node,
			TreeType* reference_node) {
   
    // Check that the pointers are not NULL
    DEBUG_ASSERT(query_node != NULL);
    DEBUG_ASSERT(reference_node != NULL);
    // Check that we really should be in the base case
    DEBUG_WARN_IF(!query_node->is_leaf());
    DEBUG_WARN_IF(!reference_node->is_leaf());

    // just checking for the single tree version
    DEBUG_ASSERT(query_node->end()
		 - query_node->begin() == 1);
    
    // Used to find the query node's new upper bound
    double query_max_neighbor_distance = -1.0;
    std::vector<std::pair<double, size_t> > neighbors(knns_);
    for (size_t query_index = query_node->begin(); 
         query_index < query_node->end(); query_index++) {
       
      // Get the query point from the matrix
      Vector query_point;
      queries_.MakeColumnVector(query_, &query_point);
      
      size_t ind = query_*knns_;
      for(size_t i=0; i<knns_; i++) {
	neighbors[i]=std::make_pair(neighbor_distances_[ind+i],
				    neighbor_indices_[ind+i]);
      }
      // We'll do the same for the references
      for (size_t reference_index = reference_node->begin(); 
           reference_index < reference_node->end(); reference_index++) {

	// Confirm that points do not identify themselves as neighbors
	// in the monochromatic case
        if (likely(reference_node != query_node ||
		   reference_index != query_index)) {
	  Vector reference_point;
	  references_.MakeColumnVector(reference_index, &reference_point);
	  // We'll use lapack to find the distance between the two vectors
	  double distance =
	    la::DistanceSqEuclidean(query_point, reference_point);
	  // If the reference point is closer than the current candidate, 
	  // we'll update the candidate
 	  if (distance < neighbor_distances_[ind+knns_-1]) {
	    neighbors.push_back(std::make_pair(distance, reference_index));
	  }
	}
      } // for reference_index

      std::sort(neighbors.begin(), neighbors.end());
      for(size_t i=0; i<knns_; i++) {
	neighbor_distances_[ind+i] = neighbors[i].first;
	neighbor_indices_[ind+i]  = neighbors[i].second;
      }
      neighbors.resize(knns_);
      // We need to find the upper bound distance for this query node
      if (neighbor_distances_[ind+knns_-1] > query_max_neighbor_distance) {
	query_max_neighbor_distance = neighbor_distances_[ind+knns_-1]; 
      }
    } // for query_index 
    // Update the upper bound for the query_node
    query_node->stat().set_max_distance_so_far(query_max_neighbor_distance);
    dc += reference_node->end() - reference_node->begin();
         
  } // ComputeBaseCase_
  
  
  /**
   * The recursive function
   */
  void ComputeNeighborsRecursion_(TreeType* query_node,
				  TreeType* reference_node, 
				  double lower_bound_distance) {

    DEBUG_ASSERT(query_node != NULL);
    DEBUG_ASSERT(reference_node != NULL);

    DEBUG_ASSERT(lower_bound_distance
		 == MinNodeDistSq_(query_node, reference_node));

    // just checking for the single tree version
    DEBUG_ASSERT(query_node->end()
		 - query_node->begin() == 1);
    
    if (lower_bound_distance > query_node->stat().max_distance_so_far()) {
      // Pruned by distance
      number_of_prunes_++;
    }
    // node->is_leaf() works as one would expect
    else if (query_node->is_leaf() && reference_node->is_leaf()) {
      // Base Case
      ComputeBaseCase_(query_node, reference_node);
    }
    else if (query_node->is_leaf()) {
      // Only query is a leaf
      
      // We'll order the computation by distance 
      double left_distance = MinNodeDistSq_(query_node,
					    reference_node->left());
      double right_distance = MinNodeDistSq_(query_node,
					     reference_node->right());
      
      if (left_distance < right_distance) {
        ComputeNeighborsRecursion_(query_node, reference_node->left(), 
				   left_distance);
        ComputeNeighborsRecursion_(query_node, reference_node->right(), 
				   right_distance);
      }
      else {
        ComputeNeighborsRecursion_(query_node, reference_node->right(), 
				   right_distance);
        ComputeNeighborsRecursion_(query_node, reference_node->left(), 
				   left_distance);
      }
    }
    
    else if (reference_node->is_leaf()) {
      // Only reference is a leaf 
      double left_distance
	= MinNodeDistSq_(query_node->left(), reference_node);
      double right_distance
	= MinNodeDistSq_(query_node->right(), reference_node);
      
      ComputeNeighborsRecursion_(query_node->left(), reference_node, 
				 left_distance);
      ComputeNeighborsRecursion_(query_node->right(), reference_node, 
				 right_distance);
      
      // We need to update the upper bound based on the new upper bounds of 
      // the children
      query_node->stat().set_max_distance_so_far(max(query_node->left()->stat().max_distance_so_far(),
						     query_node->right()->stat().max_distance_so_far()));
    } else {
      // Recurse on both as above
      
      double left_distance = MinNodeDistSq_(query_node->left(), 
					    reference_node->left());
      double right_distance = MinNodeDistSq_(query_node->left(), 
					     reference_node->right());
      
      if (left_distance < right_distance) {
        ComputeNeighborsRecursion_(query_node->left(),
				   reference_node->left(), 
				   left_distance);
        ComputeNeighborsRecursion_(query_node->left(),
				   reference_node->right(), 
				   right_distance);
      } else {
        ComputeNeighborsRecursion_(query_node->left(),
				   reference_node->right(), 
				   right_distance);
        ComputeNeighborsRecursion_(query_node->left(),
				   reference_node->left(), 
				   left_distance);
      }

      left_distance = MinNodeDistSq_(query_node->right(),
				     reference_node->left());
      right_distance = MinNodeDistSq_(query_node->right(), 
				      reference_node->right());
      
      if (left_distance < right_distance) {
        ComputeNeighborsRecursion_(query_node->right(),
				   reference_node->left(), 
				   left_distance);
        ComputeNeighborsRecursion_(query_node->right(),
				   reference_node->right(), 
				   right_distance);
      } else {
        ComputeNeighborsRecursion_(query_node->right(),
				   reference_node->right(), 
				   right_distance);
        ComputeNeighborsRecursion_(query_node->right(),
				   reference_node->left(), 
				   left_distance);
      }
      
      // Update the upper bound as above
      query_node->stat().set_max_distance_so_far(max(query_node->left()->stat().max_distance_so_far(),
						     query_node->right()->stat().max_distance_so_far()));
      
    }
  } // ComputeNeighborsRecursion_
  
  /**
   * Performs exhaustive approximate computation
   * between two nodes.
   */
  void ComputeApproxBaseCase_(TreeType* query_node,
			      TreeType* reference_node) {
   
    // Check that the pointers are not NULL
    DEBUG_ASSERT(query_node != NULL);
    DEBUG_ASSERT(reference_node != NULL);

    // This is just for now when we are dealing with
    // single trees
    DEBUG_ASSERT(query_node->end()
		 - query_node->begin() == 1);
    
    // Obtain the number of samples to be obtained
    size_t set_size
      = reference_node->end() - reference_node->begin();
    size_t sample_size = sample_sizes_[set_size - 1];
    DEBUG_ASSERT_MSG(sample_size <= set_size,
		     "n = %zu"d, N = %zu"d",
		     sample_size, set_size);

    size_t query_samples_needed
      = min_samples_per_q_ - query_node->stat().samples();

    sample_size = min(sample_size, query_samples_needed);
    DEBUG_WARN_IF(sample_size > sample_limit_);

    // Used to find the query node's new upper bound
    double query_max_neighbor_distance = -1.0;
    std::vector<std::pair<double, size_t> > neighbors(knns_);

    for (size_t query_index = query_node->begin(); 
         query_index < query_node->end(); query_index++) {
       
      // Get the query point from the matrix
      Vector query_point;
      queries_.MakeColumnVector(query_, &query_point);
      
      size_t ind = query_*knns_;
      for(size_t i=0; i<knns_; i++) {
	neighbors[i]=std::make_pair(neighbor_distances_[ind+i],
				    neighbor_indices_[ind+i]);
      }
      // We'll do the same for the references
      // but on the sample size number of points

      // Here we need to permute the reference set randomly
      for (size_t i = 0; i < sample_size; i++) {
	size_t reference_index = reference_node->begin()
	  + math::RandInt(set_size);
	DEBUG_ASSERT(reference_index < reference_node->end());

	// Confirm that points do not identify themselves as neighbors
	// in the monochromatic case
        if (likely(reference_node != query_node ||
		   reference_index != query_index)) {
	  Vector reference_point;
	  references_.MakeColumnVector(reference_index, &reference_point);
	  // We'll use lapack to find the distance between the two vectors
	  double distance =
	    la::DistanceSqEuclidean(query_point, reference_point);
	  // If the reference point is closer than the current candidate, 
	  // we'll update the candidate
 	  if (distance < neighbor_distances_[ind+knns_-1]) {
	    neighbors.push_back(std::make_pair(distance, reference_index));
	  }
	}
      } // for reference_index

      std::sort(neighbors.begin(), neighbors.end());
      for(size_t i=0; i<knns_; i++) {
	neighbor_distances_[ind+i] = neighbors[i].first;
	neighbor_indices_[ind+i]  = neighbors[i].second;
      }
      neighbors.resize(knns_);
      // We need to find the upper bound distance for this query node
      if (neighbor_distances_[ind+knns_-1] > query_max_neighbor_distance) {
	query_max_neighbor_distance = neighbor_distances_[ind+knns_-1]; 
      }
      
    } // for query_index 
    // Update the upper bound for the query_node
    query_node->stat().set_max_distance_so_far(query_max_neighbor_distance);

    // update the number of points considered and points sampled
    query_node->stat().add_total_points(set_size);
    query_node->stat().add_samples(sample_size);
  } // ComputeApproxBaseCase_

  // decides whether a reference node is small enough
  // to approximate by sampling
  inline bool is_base(TreeType* tree) {
    if (sample_sizes_[tree->end() - tree->begin() -1]
	> sample_limit_) {
      return false;
    } else {
      return true;
    }
  }

  // decides whether a query node has enough
  // samples that we can approximate the rest by
  // just picking a small number of samples
  inline bool is_almost_satisfied(TreeType* tree) {
    if (tree->stat().samples() + sample_limit_
	< min_samples_per_q_) {
      return false;
    } else {
      return true;
    }
  }

  // check if the query node has enough samples
  inline bool is_done(TreeType* tree) {
    if (tree->stat().samples() < min_samples_per_q_) {
      return false;
    } else {
      return true;
    }
  }

  /**
   * The recursive function for the approximate computation
   */
  void ComputeApproxRecursion_(TreeType* query_node,
			       TreeType* reference_node, 
			       double lower_bound_distance) {

    DEBUG_ASSERT(query_node != NULL);
    DEBUG_ASSERT(reference_node != NULL);
    // Make sure the bounding information is correct
    DEBUG_ASSERT(lower_bound_distance
		 == MinNodeDistSq_(query_node, reference_node));
    DEBUG_ASSERT(query_node->end()
		 - query_node->begin() == 1);

    if (is_done(query_node)) {
      query_node->stat().add_total_points(reference_node->end()
					  - reference_node->begin());
    } else if (lower_bound_distance
	       > query_node->stat().max_distance_so_far()) {
      // Pruned by distance
      number_of_prunes_++;

      // since we pruned this node, we can say that we encountered
      // all the points in that node
      size_t reference_size
	= reference_node->end() - reference_node->begin();
      query_node->stat().add_total_points(reference_size);
      query_node->stat().add_samples(reference_size);

    } else if (query_node->is_leaf() && reference_node->is_leaf()) {
      // Base Case
      // first check if we can do exact. If so then we do so
      // and add the number of samples encountered.
      ComputeBaseCase_(query_node, reference_node);
      size_t reference_size
	= reference_node->end() - reference_node->begin();
      query_node->stat().add_total_points(reference_size);
      query_node->stat().add_samples(reference_size);

    } else if (reference_node->is_leaf()) {
      // Only reference is a leaf 
      double left_distance
	= MinNodeDistSq_(query_node->left(), reference_node);
      double right_distance
	= MinNodeDistSq_(query_node->right(), reference_node);

      // Passing the information down to the children if it 
      // encountered some pruning earlier
      DEBUG_ASSERT_MSG(query_node->left()->stat().total_points()
		       == query_node->right()->stat().total_points(),
		       "The children of the query node should have "
		       "encountered the same number of points.");
      // if the parent has encountered extra points, pass
      // that information down to the children.
      size_t extra_points_encountered
	= query_node->stat().total_points()
	- query_node->left()->stat().total_points();
      DEBUG_ASSERT(extra_points_encountered > -1);

      if (extra_points_encountered > 0) {
	query_node->left()->stat().add_total_points(extra_points_encountered);
	query_node->right()->stat().add_total_points(extra_points_encountered);
	size_t extra_points_sampled
	  = query_node->stat().samples()
	  - min(query_node->left()->stat().samples(),
		query_node->right()->stat().samples());
	DEBUG_ASSERT(extra_points_sampled > -1);
	query_node->left()->stat().add_samples(extra_points_sampled);
	query_node->right()->stat().add_samples(extra_points_sampled);
      }

      // recurse down the query tree      
      ComputeApproxRecursion_(query_node->left(), reference_node, 
			      left_distance);
      ComputeApproxRecursion_(query_node->right(), reference_node, 
			      right_distance);
      
      // We need to update the upper bound based on the new upper bounds of 
      // the children
      query_node->stat().set_max_distance_so_far(max(query_node->left()->stat().max_distance_so_far(),
						     query_node->right()->stat().max_distance_so_far()));

      // updating the number of points considered
      // and number of samples taken

      // both the children of the query node have encountered
      // the same number of reference points. So making sure of
      // that.
      DEBUG_ASSERT_MSG(query_node->left()->stat().total_points()
		       == query_node->right()->stat().total_points(),
		       "The children of the query node should have "
		       "encountered the same number of points.");
      query_node->stat().set_total_points(query_node->left()->stat().total_points());

      // the number of samples made for each of the query points
      // is actually the minimum of both the children. And we 
      // are setting it instead of adding because we don't want
      // to have repetitions (since the information goes bottom up)
      query_node->stat().set_samples(min(query_node->left()->stat().samples(),
					 query_node->right()->stat().samples()));

    } else if (is_base(reference_node)) {
      // if the reference set is small enough to be
      // approximated by sampling.
      ComputeApproxBaseCase_(query_node, reference_node);

    } else if (is_almost_satisfied(query_node)) {
      // query node has almost enough samples,
      // just pick some samples from the reference
      // set.
      ComputeApproxBaseCase_(query_node, reference_node);

    } else if (query_node->is_leaf()) {
      // Only query is a leaf
      
      // We'll order the computation by distance 
      double left_distance = MinNodeDistSq_(query_node,
					    reference_node->left());
      double right_distance = MinNodeDistSq_(query_node,
					     reference_node->right());

      if (left_distance < right_distance) {
	ComputeApproxRecursion_(query_node, reference_node->left(), 
				left_distance);
	ComputeApproxRecursion_(query_node, reference_node->right(), 
				right_distance);
      } else {
	ComputeApproxRecursion_(query_node, reference_node->right(), 
				right_distance);
	ComputeApproxRecursion_(query_node, reference_node->left(), 
				left_distance);
      }
    } else {
      // This is the initial idea for the dual tree
      // traversal. It is an upperbound on the number
      // of points required to be samples made for
      // the particular number of points encountered
      // to maintain the probability bound for the error

      // Recurse on both as above
      double left_distance = MinNodeDistSq_(query_node->left(), 
					    reference_node->left());
      double right_distance = MinNodeDistSq_(query_node->left(), 
					     reference_node->right());
      // Passing the information down to the children if it 
      // encountered some pruning earlier
      DEBUG_ASSERT_MSG(query_node->left()->stat().total_points()
		       == query_node->right()->stat().total_points(),
		       "The children of the query node should have "
		       "encountered the same number of points.");
      // if the parent has encountered extra points, pass
      // that information down to the children.
      size_t extra_points_encountered
	= query_node->stat().total_points()
	- query_node->left()->stat().total_points();
      DEBUG_ASSERT(extra_points_encountered > -1);

      if (extra_points_encountered > 0) {
	query_node->left()->stat().add_total_points(extra_points_encountered);
	query_node->right()->stat().add_total_points(extra_points_encountered);
	size_t extra_points_sampled
	  = query_node->stat().samples()
	  - min(query_node->left()->stat().samples(),
		query_node->right()->stat().samples());
	DEBUG_ASSERT(extra_points_sampled > -1);
	query_node->left()->stat().add_samples(extra_points_sampled);
	query_node->right()->stat().add_samples(extra_points_sampled);
      }
      
      if (left_distance < right_distance) {
        ComputeApproxRecursion_(query_node->left(),
				reference_node->left(), 
				left_distance);
        ComputeApproxRecursion_(query_node->left(),
				reference_node->right(), 
				right_distance);
      } else {
        ComputeApproxRecursion_(query_node->left(),
				reference_node->right(), 
				right_distance);
        ComputeApproxRecursion_(query_node->left(),
				reference_node->left(), 
				left_distance);
      }

      left_distance = MinNodeDistSq_(query_node->right(),
				     reference_node->left());
      right_distance = MinNodeDistSq_(query_node->right(), 
				      reference_node->right());
      
      if (left_distance < right_distance) {
        ComputeApproxRecursion_(query_node->right(),
				reference_node->left(), 
				left_distance);
        ComputeApproxRecursion_(query_node->right(),
				reference_node->right(), 
				right_distance);
      } else {
        ComputeApproxRecursion_(query_node->right(),
				reference_node->right(), 
				right_distance);
        ComputeApproxRecursion_(query_node->right(),
				reference_node->left(), 
				left_distance);
      }
      
      // Update the upper bound as above
      query_node->stat().set_max_distance_so_far(max(query_node->left()->stat().max_distance_so_far(),
						     query_node->right()->stat().max_distance_so_far()));

      // both the children of the query node have encountered
      // the same number of reference points. So making sure of
      // that.
      DEBUG_ASSERT_MSG(query_node->left()->stat().total_points()
		       == query_node->right()->stat().total_points(),
		       "The children of the query node should have "
		       "encountered the same number of points.");
      query_node->stat().set_total_points(query_node->left()->stat().total_points());

      // the number of samples made for each of the query points
      // is actually the minimum of both the children. And we 
      // are setting it instead of adding because we don't want
      // to have repetitions (since the information goes bottom up)
      query_node->stat().set_samples(min(query_node->left()->stat().samples(),
					 query_node->right()->stat().samples()));
    }
  } // ComputeApproxRecursion_

  /////////////// Public Functions ////////////////////
public:
  /**
   * Setup the class and build the trees.
   * Note: we are initializing with const references to prevent 
   * local copies of the data.
   */
  void Init(const Matrix& queries_in,
	    const Matrix& references_in,
	    struct datanode* module_in) {
    
    // set the module
    module_ = module_in;
    
    // track the number of prunes
    number_of_prunes_ = 0;
    
    // Get the leaf size from the module
    leaf_size_ = fx_param_int(module_, "leaf_size", 20);
    // Make sure the leaf size is valid
    DEBUG_ASSERT(leaf_size_ > 0);
    
    // Copy the matrices to the class members since they will be rearranged.  
    queries_.Copy(queries_in);
    references_.Copy(references_in);
    
    // The data sets need to have the same number of points
    DEBUG_SAME_SIZE(queries_.n_rows(), references_.n_rows());
    
    // keep a track of the dataset
    fx_param_int(module_, "dim", queries_.n_rows());
    fx_param_int(module_, "qsize", queries_.n_cols());
    fx_param_int(module_, "rsize", references_.n_cols());

    // K-nearest neighbors initialization
    knns_ = fx_param_int(module_, "knns", 1);
  
    // Initialize the list of nearest neighbor candidates
    neighbor_indices_.Init(queries_.n_cols() * knns_);
    
    // Initialize the vector of upper bounds for each point.  
    neighbor_distances_.Init(queries_.n_cols() * knns_);
    neighbor_distances_.SetAll(DBL_MAX);

    // We'll time tree building
    fx_timer_start(module_, "tree_building");

    // This call makes each tree from a matrix, leaf size, and two arrays 
    // that record the permutation of the data points
    // Instead of NULL, it is possible to specify an array new_from_old_

    // Here we need to change the query tree into N single-point
    // query trees
    for (size_t i = 0; i < queries_.n_cols(); i++) {
      Matrix query;
      queries_.MakeColumnSlice(i, 1, &query);
      TreeType *single_point_tree
	= tree::MakeKdTreeMidpoint<TreeType>(query,
					     leaf_size_, 
					     &old_from_new_queries_,
					     NULL);
      query_trees_.push_back(single_point_tree);
      old_from_new_queries_.Renew();
    }
    reference_tree_
      = tree::MakeKdTreeMidpoint<TreeType>(references_, 
					   leaf_size_,
					   &old_from_new_references_,
					   NULL);
    
    // Stop the timer we started above
    fx_timer_stop(module_, "tree_building");

    // initializing the sample_sizes_
    sample_sizes_.Init();
  } // Init

  void Destruct() {
    for (std::vector<TreeType*>::iterator it = query_trees_.begin();
	 it < query_trees_.end(); it++) {
      if (*it != NULL) {
	delete *it;
      }
    }
    if (reference_tree_ != NULL) {
      delete reference_tree_;
    }
    queries_.Destruct();
    references_.Destruct();
    old_from_new_queries_.Renew();
    old_from_new_references_.Renew();
    neighbor_distances_.Destruct();
    neighbor_indices_.Renew();

    sample_sizes_.Renew();
  }

  /**
   * Initializes the AllNN structure for naive computation.  
   * This means that we simply ignore the tree building.
   */
  void InitNaive(const Matrix& queries_in, 
		 const Matrix& references_in,
		 size_t knns){
    
    queries_.Copy(queries_in);
    references_.Copy(references_in);
    knns_=knns;
    
    DEBUG_SAME_SIZE(queries_.n_rows(), references_.n_rows());
    
    neighbor_indices_.Init(queries_.n_cols()*knns_);
    neighbor_distances_.Init(queries_.n_cols()*knns_);
    neighbor_distances_.SetAll(DBL_MAX);
    
    // The only difference is that we set leaf_size_ to be large enough 
    // that each tree has only one node
    leaf_size_ = max(queries_.n_cols(), references_.n_cols());

    // Here we need to change the query tree into N single-point
    // query trees
    for (size_t i = 0; i < queries_.n_cols(); i++) {
      Matrix query;
      queries_.MakeColumnSlice(i, 1, &query);
      TreeType *single_point_tree
	= tree::MakeKdTreeMidpoint<TreeType>(query,
					     leaf_size_, 
					     &old_from_new_queries_,
					     NULL);
      query_trees_.push_back(single_point_tree);
      old_from_new_queries_.Renew();
    }
    
    reference_tree_
      = tree::MakeKdTreeMidpoint<TreeType>(references_,
					   leaf_size_,
					   &old_from_new_references_,
					   NULL);

    // initialiazing the sample_sizes_
    sample_sizes_.Init();
  } // InitNaive
  
  void InitApprox(const Matrix& queries_in,
		  const Matrix& references_in,
		  struct datanode* module_in) {
    
    // set the module
    module_ = module_in;

    // Check if the probability is <=1
    double alpha = fx_param_double(module_, "alpha", 1.0);
    DEBUG_ASSERT(alpha <= 1.0);
    
    // track the number of prunes
    number_of_prunes_ = 0;
    
    // Get the leaf size from the module
    leaf_size_ = fx_param_int(module_, "leaf_size", 20);
    // Make sure the leaf size is valid
    DEBUG_ASSERT(leaf_size_ > 0);

    // Getting the sample_limit
    sample_limit_ = fx_param_int(module_, "sample_limit", 20);
    
    // Copy the matrices to the class members since they will be rearranged.  
    queries_.Copy(queries_in);
    references_.Copy(references_in);
    
    // The data sets need to have the same number of points
    DEBUG_SAME_SIZE(queries_.n_rows(), references_.n_rows());
    
    // keep a track of the dataset
    fx_param_int(module_, "dim", queries_.n_rows());
    fx_param_int(module_, "qsize", queries_.n_cols());
    fx_param_int(module_, "rsize", references_.n_cols());

    // K-nearest neighbors initialization
    knns_ = fx_param_int(module_, "knns", 1);
  
    // Initialize the list of nearest neighbor candidates
    neighbor_indices_.Init(queries_.n_cols() * knns_);
    
    // Initialize the vector of upper bounds for each point.  
    neighbor_distances_.Init(queries_.n_cols() * knns_);
    neighbor_distances_.SetAll(DBL_MAX);

    // We'll time tree building
    fx_timer_start(module_, "tree_building_approx");

    // This call makes each tree from a matrix, leaf size, and two arrays 
    // that record the permutation of the data points
    // Instead of NULL, it is possible to specify an array new_from_old_

    // Here we need to change the query tree into N single-point
    // query trees
    for (size_t i = 0; i < queries_.n_cols(); i++) {
      Matrix query;
      queries_.MakeColumnSlice(i, 1, &query);
      TreeType *single_point_tree
	= tree::MakeKdTreeMidpoint<TreeType>(query,
					     leaf_size_, 
					     &old_from_new_queries_,
					     NULL);
      query_trees_.push_back(single_point_tree);
      old_from_new_queries_.Renew();
    }
    reference_tree_
      = tree::MakeKdTreeMidpoint<TreeType>(references_, 
					   leaf_size_,
					   &old_from_new_references_,
					   NULL);
    
    // Stop the timer we started above
    fx_timer_stop(module_, "tree_building_approx");

    // We will time the initialization of the sample size
    // table
    fx_timer_start(module_, "computing_sample_sizes");

    // initialize the sample_sizes array
    sample_sizes_.Init(references_.n_cols());

    // compute the sample sizes
    epsilon_ = fx_param_double(module_, "epsilon", 0.0);
    rank_approx_ = (size_t) (epsilon_
			      * (double) references_.n_cols()
			      / 100.0);

    NOTIFY("Rank Approximation: %2.3f%% or %zu"d"
	   " with Probability:%1.2f",
	   epsilon_, rank_approx_, alpha);

    ComputeSampleSizes_(rank_approx_, alpha, &sample_sizes_);

    fx_timer_stop(module_, "computing_sample_sizes");

    // initializing the minimum samples required per
    // query to hold the probability bound
    min_samples_per_q_ = sample_sizes_[references_.n_cols() -1];

  } // InitApprox
  
  /**
   * Computes the nearest neighbors and stores them in *results
   */
  void ComputeNeighbors(ArrayList<size_t>* resulting_neighbors,
                        ArrayList<double>* distances) {

    // We need to initialize the results list before filling it
    resulting_neighbors->Init(neighbor_indices_.size());
    distances->Init(neighbor_distances_.length());
    
    // Start on the root of each tree
    // the index of the query in the queries_ matrix
    query_ = 0;
    DEBUG_ASSERT((size_t)query_trees_.size() == queries_.n_cols());
    for (std::vector<TreeType*>::iterator query_tree = query_trees_.begin();
	 query_tree < query_trees_.end(); ++query_tree, ++query_) {

      ComputeNeighborsRecursion_(*query_tree, reference_tree_, 
				 MinNodeDistSq_(*query_tree,
						reference_tree_));
    }
    for (size_t i = 0; i < neighbor_indices_.size(); i++) {
      size_t query = i/knns_;
      (*resulting_neighbors)[query*knns_+ i%knns_]
	= old_from_new_references_[neighbor_indices_[i]];
      (*distances)[query*knns_+ i%knns_] = neighbor_distances_[i];
    }

    NOTIFY("Tdc = %zu"d, Tmc = %zu"d, adc = %lg, amc = %lg",
	   dc, mc, (float)dc/(float)query_trees_.size(), 
	   (float)mc/(float)query_trees_.size());
  } // ComputeNeighbors
  
  
  /**
   * Does the entire computation naively
   */
  void ComputeNaive(ArrayList<size_t>* resulting_neighbors,
                    ArrayList<double>*  distances) {

    // We need to initialize the results list before filling it
    resulting_neighbors->Init(neighbor_indices_.size());
    distances->Init(neighbor_distances_.length());
    
    // Start on the root of each tree
    // the index of the query in the queries_ matrix
    query_ = 0;
    DEBUG_ASSERT((size_t)query_trees_.size() == queries_.n_cols());
    for (std::vector<TreeType*>::iterator query_tree = query_trees_.begin();
	 query_tree < query_trees_.end(); ++query_tree, ++query_) {

      ComputeBaseCase_(*query_tree, reference_tree_);
    }
    for (size_t i = 0; i < neighbor_indices_.size(); i++) {
      size_t query = i/knns_;
      (*resulting_neighbors)[query*knns_+ i%knns_]
	= old_from_new_references_[neighbor_indices_[i]];
      (*distances)[query*knns_+ i%knns_] = neighbor_distances_[i];
    }
  }

  /**
   * Does the entire computation to find the approximate
   * rank NN
   */
  void ComputeApprox(ArrayList<size_t>* resulting_neighbors,
		     ArrayList<double>*  distances) {

    // We need to initialize the results list before filling it
    resulting_neighbors->Init(neighbor_indices_.size());
    distances->Init(neighbor_distances_.length());
    
    // Start on the root of each tree
    // the index of the query in the queries_ matrix
    query_ = 0;
    DEBUG_ASSERT((size_t)query_trees_.size() == queries_.n_cols());
    for (std::vector<TreeType*>::iterator query_tree = query_trees_.begin();
	 query_tree < query_trees_.end(); ++query_tree, ++query_) {

      ComputeApproxRecursion_(*query_tree, reference_tree_, 
			      MinNodeDistSq_(*query_tree,
					     reference_tree_));
    }
    for (size_t i = 0; i < neighbor_indices_.size(); i++) {
      size_t query = i/knns_;
      (*resulting_neighbors)[query*knns_+ i%knns_]
	= old_from_new_references_[neighbor_indices_[i]];
      (*distances)[query*knns_+ i%knns_] = neighbor_distances_[i];
    }
  }
}; //class AllkNN


#endif

