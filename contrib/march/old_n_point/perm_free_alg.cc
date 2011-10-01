/*
 *  perm_free_alg.cc
 *  
 *
 *  Created by William March on 2/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "perm_free_alg.h"


void npt::PermFreeAlg::BaseCaseHelper_(std::vector<std::vector<size_t> >& point_sets,
                                       std::vector<bool>& permutation_ok,
                                       std::vector<size_t>& points_in_tuple,
                                       int k) {
  
  std::vector<bool> permutation_ok_copy(permutation_ok);
  
  bool bad_symmetry = false;
  
  std::vector<size_t>& k_rows = point_sets[k];
  
  // iterate over possible kth members of the tuple
  for (size_t i = 0; i < k_rows.size(); i++) {
    
    size_t point_i_index = k_rows[i];
    bool this_point_works = true;
    
    bad_symmetry = false;
    
    bool i_is_random = (k < num_random_);
    
    arma::colvec vec_i;
    if (i_is_random) {
      vec_i = random_points_.col(point_i_index);
    }
    else {
      vec_i = data_points_.col(point_i_index);
    }
    
    // TODO: Does this leak memory?
    permutation_ok_copy.assign(permutation_ok.begin(), permutation_ok.end());
    
    // loop over points already in the tuple and check against them
    for (size_t j = 0; !bad_symmetry && this_point_works && j < k; j++) {
      
      size_t point_j_index = points_in_tuple[j];
      
      // j comes before i in the tuple, so it should have a lower index
      bool j_is_random = (j < num_random_);
      
      bad_symmetry = (i_is_random == j_is_random) 
                      && (point_i_index <= point_j_index);
      
      if (!bad_symmetry) {
        
        arma::colvec vec_j;
        if (j_is_random) {
          vec_j = random_points_.col(point_j_index);
        }
        else {
          vec_j = data_points_.col(point_j_index);
        }
        
        double point_dist_sq = la::DistanceSqEuclidean(vec_i, vec_j);
        
        this_point_works = matcher_.TestPointPair(point_dist_sq, j, k, 
                                                  permutation_ok_copy);
        
      } // check the distances across permutations
      
    } // for j
    
    // point i fits in the tuple
    if (this_point_works && !bad_symmetry) {
      
      points_in_tuple[k] = point_i_index;
      
      // are we finished?
      if (k == tuple_size_ - 1) {
        
        num_tuples_[num_random_]++;
        
        double this_weight = 1.0;
        
        for (int tuple_ind = 0; tuple_ind < num_random_; tuple_ind++) {
          this_weight *= random_weights_(points_in_tuple[tuple_ind]);
        }
        for (size_t tuple_ind = num_random_; tuple_ind < tuple_size_; 
             tuple_ind++) {
          
          this_weight *= data_weights_(points_in_tuple[tuple_ind]);
          
        } // iterate over the tuple
        
        weighted_num_tuples_[num_random_] += this_weight;
        
      } 
      else {
        
        BaseCaseHelper_(point_sets, permutation_ok_copy, points_in_tuple, k+1);
        
      } // need to add more points to finish the tuple
      
    } // point i fits
    
  } // for i
  
} // BaseCaseHelper_()

void npt::PermFreeAlg::BaseCase_(NodeTuple& nodes) {

  std::vector<std::vector<size_t> > point_sets(tuple_size_);
  
  // TODO: can this be done more efficiently?
  
  // Make a 2D array of the points in the nodes 
  // iterate over nodes
  for (size_t node_ind = 0; node_ind < tuple_size_; node_ind++) {
    
    point_sets[node_ind].resize(nodes.node_list(node_ind)->count());
    
    // fill in poilnts in the node
    /*
    for (size_t point_ind = nodes.node_list(node_ind)->begin(); 
         point_ind < nodes.node_list(node_ind)->end(); point_ind++) {
      
      point_sets[node_ind][point_ind] = point_ind;
      
    } // points
    */
    for (size_t i = 0; i < nodes.node_list(node_ind)->count(); i++) {
      
      point_sets[node_ind][i] = i + nodes.node_list(node_ind)->begin();
      
    }
    
  } // nodes
  
  std::vector<bool> permutation_ok(num_permutations_, true);
  
  std::vector<size_t> points_in_tuple(tuple_size_, -1);
  
  BaseCaseHelper_(point_sets, permutation_ok, points_in_tuple, 0);
  
} // BaseCase_()

bool npt::PermFreeAlg::CanPrune_(NodeTuple& nodes) {
  
  return !(matcher_.TestNodeTuple(nodes));
  
} // CanPrune

void npt::PermFreeAlg::DepthFirstRecursion_(NodeTuple& nodes) {
  
  if (CanPrune_(nodes)) {
  //if (false) {
    num_prunes_++;
    
  }
  //else if (true) {
  else if (nodes.all_leaves()) {
    
    BaseCase_(nodes);
    num_base_cases_++;
    
  } 
  else {
    
    // split nodes and call recursion

    // left child
    if (nodes.CheckSymmetry(nodes.ind_to_split(), true)) {
      // do left recursion
      
      NodeTuple left_child(nodes, true);
      DepthFirstRecursion_(left_child);
      
    }
    else {
      num_prunes_++;
    }
    // right child
    if (nodes.CheckSymmetry(nodes.ind_to_split(), false)) {
    
      NodeTuple right_child(nodes, false);
      DepthFirstRecursion_(right_child);
      
    }
    else {
      num_prunes_++;
    }
    
  } // recurse 
  
} // DepthFirstRecursion
