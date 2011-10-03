/*
 *  multi_matcher.h
 *  
 *
 *  Created by William March on 6/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


// Idea: for each of the (n choose 2) distances in the matcher, the user will 
// specify a range and number of distances to compute for 


// IMPORTANT: assuming that all dimensions have the same thickness
// assuming that matcher values +- band don't overlap within a dimension

#ifndef MULTI_MATCHER_H
#define MULTI_MATCHER_H

#include "permutations.h"
#include "node_tuple.h"

namespace npt {
  
  class MultiMatcher {
    
  private:

    
    arma::mat& data_mat_;
    arma::colvec& data_weights_;
    
    arma::mat& random_mat_;
    arma::colvec& random_weights_;
    
    
    // first index: num_random_
    // second index: matcher_ind_0 + num_bands[0]*matcher_ind_1 + . . .
    std::vector<std::vector<int> > results_;
    std::vector<std::vector<double> > weighted_results_;
    
    
    // for now, I'm assuming a single, global thickness for each dimension of 
    // the matcher
    double bandwidth_;
    double half_band_;
    int total_matchers_;
    
    int tuple_size_;
    
    int n_choose_2_;
    
    // IMPORTANT: need an ordering of these
    
    // all these need length (n choose 2)
    // these are the max and min of the range for each dimension
    std::vector<double> min_bands_sq_;
    std::vector<double> max_bands_sq_;
    
    // entry i,j is the jth matcher value in dimension i
    std::vector<std::vector<double> > matcher_dists_; 
    
    /*
    // (i,j) is true if matcher dimensions i and j are equal
    std::vector<std::vector<bool> > matcher_dim_equal_;
    */
    
    // the number of bandwidths in each dimension
    std::vector<int> num_bands_;
    
    // these are just min_bands and max bands sorted (and squared)
    std::vector<double> upper_bounds_sq_;
    std::vector<double> lower_bounds_sq_;
    
    Permutations perms_;
    int num_permutations_;
    int num_random_;
    
    
    // We want the matcher dimension that is the distance between point i and j
    size_t IndexMatcherDim_(size_t i, size_t j);
    
    size_t GetPermIndex_(size_t perm_index, size_t pt_index) {
      return perms_.GetPermutation(perm_index, pt_index);
    } // GetPermIndex_
    
    void BaseCaseHelper_(std::vector<std::vector<size_t> >& point_sets,
                         std::vector<bool>& permutation_ok,
                         std::vector<std::vector<size_t> >& perm_locations,
                         std::vector<size_t>& points_in_tuple, int k);
    
    
  public:
    
    MultiMatcher(const std::vector<double>& min_bands, 
                 const std::vector<double>& max_bands,
                 const std::vector<int>& num_bands, 
                 const double band, size_t tuple_size) : num_bands_(num_bands),
    perms_(tuple_size), tuple_size_(tuple_size), results_(tuple_size+1),
    weighted_results_(tuple_size+1)
    {

      total_matchers_ = 1;
      for (size_t i = 0; i < num_bands.size(); i++) {
        total_matchers_ *= num_bands[i];
      }
      for (int i = 0; i <= tuple_size_; i++) {
        results_[i].resize(total_matchers_, 0);
        weighted_results_[i].resize(total_matchers_, 0.0);
      }      
      
      
      bandwidth_ = band;
      half_band_ = bandwidth_ / 2.0;
      
      min_bands_sq_.resize(min_bands.size());
      max_bands_sq_.resize(max_bands.size());

      upper_bounds_sq_.resize(min_bands.size());
      lower_bounds_sq_.resize(max_bands.size());
      
      for (size_t i = 0; i < max_bands.size(); i++) {
        
        min_bands_sq_[i] = min_bands[i] * min_bands[i];
        max_bands_sq_[i] = max_bands[i] * max_bands[i];
        
        if (min_bands[i] - half_band_ > 0) {
          lower_bounds_sq_[i] = (min_bands[i] - half_band_) 
                                * (min_bands[i] - half_band_);
        
        }
        else {
          lower_bounds_sq_[i] = 0.0;
        }
          upper_bounds_sq_[i] = (max_bands[i] + half_band_) 
                              * (max_bands[i] + half_band_);
        
      }
      
      std::sort(lower_bounds_sq_.begin(), lower_bounds_sq_.end());
      std::sort(upper_bounds_sq_.begin(), upper_bounds_sq_.end());
      
      n_choose_2_ = num_bands_.size();
      matcher_dists_.resize(n_choose_2_);
      
      for (size_t i = 0; i < n_choose_2_; i++) {
        
        double band_step = (max_bands[i] - min_bands[i]) / ((double)num_bands_[i] - 1.0);
        
        matcher_dists_[i].resize(num_bands_[i]);
        
        if (num_bands_[i] > 1) {
          for (size_t j = 0; j < num_bands_[i]; j++) {
            
            matcher_dists_[i][j] = min_bands[i] + (double)j * band_step;
            
          } // for j
        }
        else {
          matcher_dists_[i][0] = min_bands[i];
        }
      } // for i

      
      num_permutations_ = perms_.num_permutations();
      
    } // constructor
    
    
    bool TestPointPair(double dist_sq, size_t new_ind, size_t old_ind,
                       std::vector<bool>& permutation_ok,
                       std::vector<std::vector<size_t> >&perm_locations);
    
    bool TestNodeTuple(NodeTuple& nodes);
    
    void BaseCase(NodeTuple& nodes);
    
    void set_num_random(int n) {
      num_random_ = n;
    }
    
    std::vector<std::vector<int> >& results() {
      return results_;
    }
    
    std::vector<std::vector<double> >& weighted_results() {
      return weighted_results_;
    }
    
    
    int num_permutations() {
      return perms_.num_permutations(); 
    }
    
    double matcher_dists(size_t i, size_t j) {
      return (matcher_dists_[i][j]);
    }
    
  }; // class
  
} // namespace

#endif 
