/*
 *  permutations.h
 *  
 *
 *  Created by William March on 2/7/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 *  Stores all the permutations of n elements.  Used in the standard 
 *  multi-tree algorithm.
 *
 */

#ifndef PERMUTATCLINS_H
#define PERMUTATCLINS_H

#include <mlpack/core.h>

namespace npt {

  class Permutations {
    
  private:
    
    ///////////// member variables ///////////////////////
    
    
    // permutation_indices_(i, j) is the location of point i in permutation j
    arma::Mat<int> permutation_indices_;
    
    // the value of n being considered
    int tuple_size_;
    
    // tuple_size_!
    int num_perms_;
    
    
    //////////////// functions //////////////////
    
    // helper function at startup, actually forms all the permutations
    void GeneratePermutations_(int k, int* perm_index,
                               arma::Col<int>& trial_perm);
    
    
    
    
  public:
    
    // Dummy empty constructor
    Permutations() {}
    
    // The constructor needs to fill in permuation_indices_
    Permutations(size_t n) {

      tuple_size_ = n;
      
      // compute the factorial
      num_perms_ = 1;
      for (int i = 2; i <= tuple_size_; i++) {
        num_perms_ = num_perms_ * i;
      } // for i
      
      // make the trial permutation
      arma::Col<int> trial_perm(tuple_size_);
      trial_perm.fill(-1);
      
      // allocate the matrix
      permutation_indices_.set_size(tuple_size_, num_perms_);
      permutation_indices_.fill(-1);
      //permutation_indices_(tuple_size_, 0);
      
      //std::cout << "n_rows: " << permutation_indices_.n_rows;
      //std::cout << " n_cols: " << permutation_indices_.n_cols << "\n";
      //std::cout << "trial_perm: " << trial_perm.n_rows << "\n";
      
      int perm_index = 0;
      //Print();
      GeneratePermutations_(0, &perm_index, trial_perm);
      
    } // constructor
    
    int num_permutations() const {
      return num_perms_;
    }
    
    // just accesses elements of permutation_indices_
    int GetPermutation(size_t perm_index, size_t point_index) const {
      
      // note that these are backward from how they're input
      // the old code does it this way and I don't want to get confused
      return permutation_indices_(point_index, perm_index);
      
    } // GetPermutation()
    
    
    // for debugging purposes
    void Print() {
      
      permutation_indices_.print("Permutation Indices:");
      
    }
    
    
  }; // class
  
} // namespace

#endif

