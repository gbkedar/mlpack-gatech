/*
 *  n_point_main.cc
 *  
 *
 *  Created by William March on 2/7/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "fastlib/fastlib.h"
#include <fastlib/fx/io.h>
#include<log.h>

//#include "single_bandwidth_alg.h"
#include "naive_alg.h"
//#include "perm_free_alg.h"
//#include "multi_bandwidth_alg.h"


PARAM_STRING_REQ("data", "Point coordinates.", NULL);
PARAM_STRING_REQ("random", "Poisson set coordinates.", NULL);
PARAM_FLAG("weighted_computation", "Specify if computing with pointwise weights", NULL)
PARAM_STRING("weights", "Optional data weights.", NULL, "default_weights.csv");
PARAM_STRING("random_weights", "Optional weights on Poisson set.", NULL, "default_weights.csv");
PARAM_STRING_REQ("matcher_dists", "The distances in the matcher, stored in a symmetric matrix.",
                 NULL);
PARAM_DOUBLE("bandwidth", "Thickness of the matcher", NULL,
      1.0)
PARAM_INT("leaf_size", "Max number of points in a leaf node", NULL, 1);
PARAM_FLAG("do_naive", "Permform Naive computation", NULL);
PARAM_FLAG("do_single_bandwidth", "Permform old (Moore & Gray) tree computation", NULL);
PARAM_FLAG("do_perm_free", "Tree computation with alternative pruning rule", NULL);

using namespace mlpack;
using namespace npt;

int main(int argc, char* argv[]) {

  //fx_init(argc, argv, NULL);
  IO::ParseCommandLine(argc, argv);
  
  // read in data and parameters
  
  //std::string data_filename = fx_param_str(NULL, "data", "test_npt_pts.csv");
  std::string data_filename = IO::GetParam<std::string>("data");
  
  arma::mat data_in, data_mat;
  data_in.load(data_filename, arma::raw_ascii);
  
  // THIS IS BAD: do it better
  if (data_in.n_rows > data_mat.n_cols) {
    data_mat = arma::trans(data_in);
  }
  else {
    data_mat = data_in;
  }
  
  //arma::mat data_out = arma::trans(data_mat);
  //data_out.save("3pt_test_data.csv", arma::raw_ascii);
  
  arma::colvec weights;  
  //if (fx_param_exists(NULL, "weights")) {
  if (IO::HasParam("weighted_computation")) {
    weights.load(IO::GetParam<std::string>("weights"));
  }
  else {
    weights.set_size(data_mat.n_cols);
    weights.fill(1.0);
  }
  
  
  std::string random_filename = IO::GetParam<std::string>("random");
  
  arma::mat random_in, random_mat;
  random_in.load(random_filename, arma::raw_ascii);
  
  // THIS IS BAD: do it better
  if (random_in.n_rows > random_mat.n_cols) {
    random_mat = arma::trans(random_in);
  }
  else {
    random_mat = random_in;
  }
  
  //arma::mat data_out = arma::trans(data_mat);
  //data_out.save("3pt_test_data.csv", arma::raw_ascii);
  
  arma::colvec random_weights;  
  //if (fx_param_exists(NULL, "weights")) {
  if (IO::HasParam("weighted_computation")) {
    random_weights.load(IO::GetParam<std::string>("random_weights"));
  }
  else {
    random_weights.set_size(random_mat.n_cols);
    random_weights.fill(1.0);
  }
  
  
  //std::cout << "loaded weights\n";
  
  arma::mat matcher_dists;
  //matcher_dists.load(fx_param_str(NULL, "matcher_dists", 
  //                                "test_matcher_dists.csv"));
  matcher_dists.load(IO::GetParam<std::string>("matcher_dists"));
  //double bandwidth = fx_param_double(NULL, "bandwidth", 0.05);
  double bandwidth = IO::GetParam<double>("bandwidth");
  
  //std::cout << "loaded bounds\n";
  
  
  // run algorithm
  
  //if (fx_param_exists(NULL, "do_naive")) {
  if (IO::HasParam("do_naive")) {
    //std::cout << "Doing naive.\n";
    
    Log::Info << "Doing naive." << std::endl;
    
    //fx_timer_start(NULL, "naive_time");
    IO::StartTimer("naive_time");
    
    NaiveAlg naive_alg(data_mat, weights, random_mat, 
                       random_weights, matcher_dists, bandwidth);
    
    naive_alg.ComputeCounts();
    
    //fx_timer_stop(NULL, "naive_time");
    IO::StopTimer("naive_time");
    
    //std::cout << "\nNaive num tuples: " << naive_alg.num_tuples() << "\n\n";
    Log::Info << std::endl << "Naive num tuples: " << std::endl;
    
    naive_alg.print_num_tuples();
    
    Log::Info << std::endl << std::endl;
    
  } // do naive
  
  /*
  
  //size_t leaf_size = fx_param_int(NULL, "leaf_size", 1);
  size_t leaf_size = (size_t)IO::GetParam<int>("leaf_size");
  
  //if (fx_param_exists(NULL, "do_single_bandwidth")) {
  if (IO::HasParam("do_single_bandwidth")) {
    
    //std::cout << "Doing single bandwidth.\n";
    Log::Info << "Doing single bandwidth.\n";

    //fx_timer_start(NULL, "single_bandwidth_time");
    IO::StartTimer("single_bandwidth_time");
    
    SingleBandwidthAlg single_alg(data_mat, weights, random_mat, 
                                  random_weights, leaf_size, 
                                  matcher_dists, bandwidth);
    
    single_alg.ComputeCounts();
    
    //fx_timer_stop(NULL, "single_bandwidth_time");
    IO::StopTimer("single_bandwidth_time");
    
    //std::cout << "\nSingle Bandwidth num tuples: " << single_alg.num_tuples() << "\n\n";
    Log::Info << std::endl << "Single bandwidth num tuples: " << std::endl;
    
    single_alg.print_num_tuples();
    
    Log::Info << std::endl << std::endl;
    
  } // single bandwidth
  
  
  //if (fx_param_exists(NULL, "do_perm_free")) {
  if (IO::HasParam("do_perm_free")) {
    
    Log::Info << "Doing permutation free.\n";

    //fx_timer_start(NULL, "perm_free_time");
    IO::StartTimer("perm_free_time");
    
    PermFreeAlg alg(data_mat, weights, random_mat, random_weights, leaf_size, 
                    matcher_dists, bandwidth);
    
    alg.Compute();
    
    //fx_timer_stop(NULL, "perm_free_time");
    IO::StopTimer("perm_free_time");
    
    Log::Info << "\nPerm Free num tuples: " << std::endl;
    alg.print_num_tuples();
    Log::Info << std::endl << std::endl;
    
    
  } // perm free
  */

  //fx_done(NULL);
  
  return 0;
  
} // main()
