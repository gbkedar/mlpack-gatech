/** @file fast_multipole_method.h
 *
 *  This file contains an implementation of fast multipole method for
 *  the general $\frac{1}{r^{\lambda}}$ potential function. It uses an
 *  adpative octree implementation.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 *  @see fast_multipole_method_main.cc
 *  @bug No known bugs.
 */

#ifndef FAST_MULTIPOLE_METHOD_H
#define FAST_MULTIPOLE_METHOD_H

#include "fastlib/fastlib.h"
#include "fmm_stat.h"
#include "mlpack/series_expansion/inverse_pow_dist_kernel.h"
#include "mlpack/series_expansion/inverse_pow_dist_farfield_expansion.h"
#include "mlpack/series_expansion/inverse_pow_dist_local_expansion.h"
#include "contrib/dongryel/multitree_template/multitree_utility.h"
#include "contrib/dongryel/proximity_project/gen_hypercube_tree.h"
#include "contrib/dongryel/proximity_project/gen_hypercube_tree_util.h"

class FastMultipoleMethod {

 private:

  ////////// Private Member Variables //////////

  double lambda_;

  /** @brief The pointer to the module holding the parameters.
   */
  struct datanode *module_;

  /** @brief The boolean flag to control the leave-one-out computation.
   */
  bool leave_one_out_;

  /** @brief The inverse distance kernel object.
   */
  InversePowDistKernel kernel_;

  /** @brief The series expansion auxilary object.
   */
  InversePowDistSeriesExpansionAux sea_;

  /** @brief The shuffled query particle set.
   */
  Matrix shuffled_query_particle_set_;

  /** @brief The shuffled reference particle set.
   */
  Matrix shuffled_reference_particle_set_;

  /** @brief The shuffled reference particle charge set.
   */
  Vector shuffled_reference_particle_charge_set_;

  /** @brief The octree containing the entire particle set.
   */
  proximity::GenHypercubeTree<FmmStat> *tree_;

  /** @brief The list of nodes on each level.
   */
  ArrayList< ArrayList <proximity::GenHypercubeTree<FmmStat> *> > nodes_in_each_level_;

  /** @brief The number of query particles in the particle set.
   */
  int num_query_particles_;
  
  /** @brief The number of reference particles in the particle set.
   */
  int num_reference_particles_;

  /** @brief The permutation mapping indices of the particle indices
   *         to original order.
   */
  ArrayList< ArrayList<size_t> > old_from_new_index_;

  /** @brief The permutation mapping indices of the shuffled indices
   *         from the original order.
   */
  ArrayList< ArrayList<size_t> > new_from_old_index_;

  /** @brief The accumulated potential for each query particle.
   */
  Vector potentials_;

  ////////// Private Member Functions //////////

  void ReshuffleResults_(Vector &to_be_reshuffled) {

    size_t query_point_indexing = 
      (shuffled_reference_particle_set_.ptr() == 
       shuffled_query_particle_set_.ptr()) ? 0:1;

    // Reshuffle the results to account for dataset reshuffling
    // resulted from tree constructions.
    Vector tmp_results;
    tmp_results.Init(to_be_reshuffled.length());
    
    for(size_t i = 0; i < tmp_results.length(); i++) {
      tmp_results[old_from_new_index_[query_point_indexing][i]] =
	to_be_reshuffled[i];
    }
    for(size_t i = 0; i < tmp_results.length(); i++) {
      to_be_reshuffled[i] = tmp_results[i];
    }
  }

  void FormMultipoleExpansions_() {
    
    Vector node_center;
    node_center.Init(shuffled_reference_particle_set_.n_rows());

    // Start from the most bottom level, and work your way up to the
    // direct children of the root node.
    for(size_t level = nodes_in_each_level_.size() - 1; level >= 0; level--) {

      // The references to the nodes on the current level.
      ArrayList<proximity::GenHypercubeTree<FmmStat> *> 
	&nodes_on_current_level = nodes_in_each_level_[level];

      // Iterate over each node in the list.
      for(size_t n = 0; n < nodes_on_current_level.size(); n++) {
	
	proximity::GenHypercubeTree<FmmStat> *node = nodes_on_current_level[n];
	
	// Compute the node center.
	for(size_t i = 0; i < shuffled_reference_particle_set_.n_rows(); 
	    i++) {
	  node_center[i] = 0.5 * (node->bound().get(i).lo +
				  node->bound().get(i).hi);
	}

	// Initialize the far-field expansion of the current node.
	node->stat().farfield_expansion_.Init(node_center, &sea_);

	// Also initialize the local expansion of the current node (to
	// be used in the downward pass later).
	node->stat().local_expansion_.Init(node_center, &sea_);

	// If the current node is a leaf node, then compute
	// exhaustively its far-field moments.
	if(node->is_leaf()) {
	  node->stat().farfield_expansion_.AccumulateCoeffs
	    (shuffled_reference_particle_set_,
	     shuffled_reference_particle_charge_set_,
	     node->begin(0), node->end(0), sea_.get_max_order());
	}
	
	// Otherwise, translate the moments owned by the children in a
	// bottom-up fashion.
	else {
	  for(size_t child = 0; child < node->num_children(); child++) {
	    node->stat().farfield_expansion_.TranslateFromFarField
	      (node->get_child(child)->stat().farfield_expansion_);
	  }
	}
      }
    }
  }

  void EvaluateMultipoleExpansion_
  (proximity::GenHypercubeTree<FmmStat> *query_node, 
   proximity::GenHypercubeTree<FmmStat> *reference_node) {

    size_t query_point_indexing = 
      (shuffled_reference_particle_set_.ptr() == 
       shuffled_query_particle_set_.ptr()) ? 0:1;

    for(size_t q = query_node->begin(query_point_indexing); 
	q < query_node->end(query_point_indexing); q++) {
      
      potentials_[q] += 
	reference_node->stat().farfield_expansion_.EvaluateField
	(shuffled_query_particle_set_, q, sea_.get_max_order());
    }
  }

  void BaseCase_(proximity::GenHypercubeTree<FmmStat> *query_node,
		 proximity::GenHypercubeTree<FmmStat> *reference_node,
		 Vector &potentials) {
    
    size_t query_point_indexing = 
      (shuffled_reference_particle_set_.ptr() == 
       shuffled_query_particle_set_.ptr()) ? 0:1;

    for(size_t q = query_node->begin(query_point_indexing); 
	q < query_node->end(query_point_indexing); q++) {
      
      // Get the query point.
      const double *q_col = shuffled_query_particle_set_.GetColumnPtr(q);

      for(size_t r = reference_node->begin(0); r < reference_node->end(0);
	  r++) {
	
	// Compute the pairwise distance, if the query and the
	// reference are not the same particle.
	if(leave_one_out_ && q == r) {
	  continue;
	}
	const double *r_col = shuffled_reference_particle_set_.GetColumnPtr(r);
	
	double sq_dist = la::DistanceSqEuclidean
	  (shuffled_query_particle_set_.n_rows(), q_col, r_col);
	
	potentials[q] += shuffled_reference_particle_charge_set_[r] *
	  kernel_.EvalUnnormOnSq(sq_dist);
      }
    }
  }

  void EvaluateLocalExpansion_
  (proximity::GenHypercubeTree<FmmStat> *query_node) {

    size_t query_point_indexing = 
      (shuffled_reference_particle_set_.ptr() == 
       shuffled_query_particle_set_.ptr()) ? 0:1;

    for(size_t q = query_node->begin(query_point_indexing); 
	q < query_node->end(query_point_indexing); q++) {

      // Evaluate the local expansion at the current query point.
      potentials_[q] += query_node->stat().local_expansion_.
	EvaluateField(shuffled_query_particle_set_, q,
		      sea_.get_max_order());
    }    
  }

  void TransmitLocalExpansionToChildren_
  (proximity::GenHypercubeTree<FmmStat> *query_node) {
    
    for(size_t c = 0; c < query_node->num_children(); c++) {
      
      // Query child.
      proximity::GenHypercubeTree<FmmStat> *query_child_node =
	query_node->get_child(c);
      
      query_node->stat().local_expansion_.TranslateToLocal
	(query_child_node->stat().local_expansion_);
    }
  }

  void DownwardPass_() {

    size_t query_point_indexing = 
      (shuffled_reference_particle_set_.ptr() ==
       shuffled_query_particle_set_.ptr()) ? 0:1;

    // Start from the top level and descend down the tree.
    for(size_t level = 1; level < nodes_in_each_level_.size(); level++) {
      
      // Retrieve the nodes on the current level.
      const ArrayList<proximity::GenHypercubeTree<FmmStat> * > 
	&nodes_on_current_level = nodes_in_each_level_[level];
      
      // Iterate over each node in this level.
      for(size_t n = 0; n < nodes_on_current_level.size(); n++) {

	// The pointer to the current query node.
	proximity::GenHypercubeTree<FmmStat> *node = nodes_on_current_level[n];

	// If the node does not contain any query points, then skip
	// it.
	if(node->count(query_point_indexing) == 0) {
	  continue;
	}

	// Compute the colleague nodes of the given node. This
	// corresponds to Cheng, Greengard, and Rokhlin's List 2 in
	// their description of the algorithm.
	ArrayList<proximity::GenHypercubeTree<FmmStat> *> colleagues;
	GenHypercubeTreeUtil::FindColleagues
	  (shuffled_query_particle_set_.n_rows(), node, nodes_in_each_level_,
	   &colleagues);

	// Perform far-to-local translation for the colleague nodes.
	for(size_t c = 0; c < colleagues.size(); c++) {

	  proximity::GenHypercubeTree<FmmStat> *colleague_node = colleagues[c];

	  if(colleague_node->count(0) > 0) {
	    colleague_node->stat().farfield_expansion_.TranslateToLocal
	      (node->stat().local_expansion_, sea_.get_max_order());
	  }
	  
	} // end of iterating over each colleague...
	 
	// These correspond to the List 1 and List 3 of the same
	// paper.
	ArrayList<proximity::GenHypercubeTree<FmmStat> *> adjacent_leaves;
	ArrayList<proximity::GenHypercubeTree<FmmStat> *> 
	  non_adjacent_children;

	// If the current query node is a leaf node, then compute List
	// 1 and List 3 of the Cheng/Greengard/Rokhlin paper.
	if(node->is_leaf()) {
	  	  
	  GenHypercubeTreeUtil::FindAdjacentLeafNode
	    (shuffled_query_particle_set_.n_rows(), nodes_in_each_level_, node,
	     &adjacent_leaves, &non_adjacent_children);

	  // Iterate over each node in List 1 and directly compute the
	  // contribution.
	  for(size_t adjacent = 0; adjacent < adjacent_leaves.size(); 
	      adjacent++) {
	    
	    proximity::GenHypercubeTree<FmmStat> *reference_leaf_node =
	      adjacent_leaves[adjacent];
	    
	    DEBUG_ASSERT(reference_leaf_node->is_leaf());
	    if(reference_leaf_node->count(0) > 0) {
	      BaseCase_(node, reference_leaf_node, potentials_);
	    }

	  } // end of iterating over List 1...

	  // Iterate over each node in List 3 and directly evaluate
	  // its far-field expansion.
	  for(size_t non_adjacent = 0; non_adjacent < 
		non_adjacent_children.size(); non_adjacent++) {
	    proximity::GenHypercubeTree<FmmStat> *reference_node =
	      non_adjacent_children[non_adjacent];

	    // This is the cut-off that determines whether exhaustive
	    // base case of the direct far-field evaluation is
	    // cheaper.
	    
	    if(reference_node->count(0) > 0) {
	      if(reference_node->count(0) > 
		 sea_.get_max_order() * sea_.get_max_order() * 
		 sea_.get_max_order()) {
		EvaluateMultipoleExpansion_(node, reference_node);
	      }
	      else {
		BaseCase_(node, reference_node, potentials_);
	      }
	    }
	    
	  } // end of iterating over List 3...
	}
	else {
	  adjacent_leaves.Init();
	  non_adjacent_children.Init();	  
	}

	// Compute List 4.
	ArrayList<proximity::GenHypercubeTree<FmmStat> * > fourth_list;
	GenHypercubeTreeUtil::FindFourthList
	  (nodes_in_each_level_, node->node_index(), node->level(),
	   shuffled_query_particle_set_.n_rows(), adjacent_leaves, 
	   colleagues, non_adjacent_children, &fourth_list);
	
	// Directly accumulate the contribution of each reference node
	// in List 4.
	for(size_t direct_accum = 0; direct_accum < fourth_list.size();
	    direct_accum++) {
	  
	  proximity::GenHypercubeTree<FmmStat> *reference_node =
	    fourth_list[direct_accum];
	  
	  // This is the cut-off that determines whether computing by
	  // direct accumulation is cheaper with respect to the base
	  // case method.
	  if(reference_node->count(0) > 0) {
	    if(node->count(query_point_indexing) >
	       sea_.get_max_order() * sea_.get_max_order() * 
	       sea_.get_max_order()) {
	      
	      node->stat().local_expansion_.AccumulateCoeffs
		(shuffled_reference_particle_set_,
		 shuffled_reference_particle_charge_set_,
		 reference_node->begin(0), reference_node->end(0),
		 sea_.get_max_order());
	    }
	    else {
	      BaseCase_(node, reference_node, potentials_);
	    }
	  }
	}
	
	// If the current query node is a leaf node, then we have to
	// evaluate its local expansion, plus the self-interaction!
	if(node->is_leaf()) {
	  EvaluateLocalExpansion_(node);

	  // If the node contains any reference points, then we have
	  // to do the self-interactions among the node.
	  if(node->count(0) > 0) {
	    BaseCase_(node, node, potentials_);
	  }
	}
	
	// Otherwise, we need to pass it down.
	else {
	  TransmitLocalExpansionToChildren_(node);
	}

      } // end of iterating over each query box node on this level...
      
    } // end of iterating over each level...
  }

  void OutputResultsToFile_(const Vector &results, const char *fname) {

    FILE *stream = fopen(fname, "w+");
    for(size_t q = 0; q < results.length(); q++) {
      fprintf(stream, "%g\n", results[q]);
    }    
    fclose(stream);
  }

 public:

  FastMultipoleMethod() {
  }

  ~FastMultipoleMethod() {
    if(tree_ != NULL) {
      delete tree_;
      tree_ = NULL;
    }
  }

  void NaiveCompute(Vector *naively_computed_potentials) {
    
    printf("Starting the naive computation...\n");

    naively_computed_potentials->Init(shuffled_query_particle_set_.n_cols());

    fx_timer_start(NULL, "naive_fmm_compute");

    // Call the base case...
    naively_computed_potentials->SetZero();
    BaseCase_(tree_, tree_, *naively_computed_potentials);

    fx_timer_stop(NULL, "naive_fmm_compute");

    printf("Finished the naive computation...\n");

    // Reshuffle the results according to the permutation.
    ReshuffleResults_(*naively_computed_potentials);

    // Output the results to the file.
    OutputResultsToFile_(*naively_computed_potentials, "naive_fmm_output.txt");
  }

  void Compute() {
    
    printf("Starting the computation...\n");

    fx_timer_start(NULL, "fmm_compute");

    // Reset the accumulated sum.
    potentials_.SetZero();

    // Upward pass: Form multipole expansions.
    FormMultipoleExpansions_();

    // Downward pass
    if(tree_->is_leaf()) {
      BaseCase_(tree_, tree_, potentials_);
    }
    else {
      DownwardPass_();
    }

    fx_timer_stop(NULL, "fmm_compute");

    printf("Finished the computation...\n");

    // Reshuffle the results to account for dataset reshuffling
    // resulted from tree constructions.
    ReshuffleResults_(potentials_);

    // Output the results to the file.
    OutputResultsToFile_(potentials_, "fast_fmm_output.txt");
  }

  void Init(const Matrix &queries, const Matrix &references,
	    const Matrix &rset_weights, bool queries_equal_references, 
	    struct datanode *module_in) {
    
    // Point to the incoming module.
    module_ = module_in;
    
    // Set the flag for whether to perform leave-one-out computation.
    leave_one_out_ = (queries.ptr() == references.ptr());

    // Read in the number of points owned by a leaf.
    int leaflen = std::max((long long int) 3, 
			   fx_param_int(module_in, "leaflen", 40));

    // Set the number of query particles and reference particles
    // accordingly.
    num_query_particles_ = queries.n_cols();
    num_reference_particles_ = references.n_cols();
    
    // Approporiately initialize the query/reference sets.
    ArrayList<Matrix *> particle_sets;
    particle_sets.Init();
    shuffled_reference_particle_set_.Copy(references);
    *(particle_sets.PushBackRaw()) = &shuffled_reference_particle_set_;

    if(queries.ptr() != references.ptr()) {
      shuffled_query_particle_set_.Copy(queries);
      *(particle_sets.PushBackRaw()) = &shuffled_query_particle_set_;
    }
    else {
      shuffled_query_particle_set_.Alias(shuffled_reference_particle_set_);
    }

    // Copy over the reference charge set.
    shuffled_reference_particle_charge_set_.Init(rset_weights.n_cols());
    for(size_t i = 0; i < rset_weights.n_cols(); i++) {
      shuffled_reference_particle_charge_set_[i] = rset_weights.get(0, i);
    }

    // Construct query and reference trees. Shuffle the reference
    // weights according to the permutation of the reference set in
    // the reference tree.
    fx_timer_start(NULL, "tree_d");
    int max_tree_depth = fx_param_int(module_, "max_tree_depth", 3);
    tree_ = proximity::MakeGenHypercubeTree(particle_sets, leaflen,
					    max_tree_depth,
					    &nodes_in_each_level_,
					    &old_from_new_index_,
					    &new_from_old_index_);
    fx_timer_stop(NULL, "tree_d");

    printf("Constructed the tree...\n");
    
    // Shuffle the reference particle charges according to the
    // permutation of the reference particle set.
    MultiTreeUtility::ShuffleAccordingToPermutation
      (shuffled_reference_particle_charge_set_, old_from_new_index_[0]);

    // Retrieve the lambda order needed for expansion.
    lambda_ = fx_param_double(module_, "lambda", 1.0);

    // Initialize the kernel.
    kernel_.Init(lambda_, queries.n_rows());

    // Initialize the series expansion auxliary object.
    sea_.Init(lambda_, fx_param_int(module_, "order", 5), references.n_rows());

    // Allocate the vector for storing the accumulated potential.
    potentials_.Init(shuffled_query_particle_set_.n_cols());
  }
};

#endif
