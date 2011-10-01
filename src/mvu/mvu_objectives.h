/*
 * =====================================================================================
 * 
 *       Filename:  mvu_objectives.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  03/05/2008 12:00:56 PM EST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

#ifndef MVU_OBJECTIVES_H_
#define MVU_OBJECTIVES_H_

#include <fastlib/fastlib.h>
#include <mlpack/allknn/allknn.h>
#include <mlpack/allkfn/allkfn.h>
#include <fastlib/optimization/lbfgs/optimization_utils.h>

PARAM_INT_REQ("new_dimension", "the number of dimensions for the unfolded", 
  "optfun");
PARAM_STRING("nearest_neighbor_file", "file with the nearest neighbor pairs\
 and the squared distances defaults to nearest.txt", "optfun", "nearest.txt");
PARAM_STRING("furthest_neighbor_file", "file with the nearets neighbor\
 pairs and the squared distances", "optfun", "furthest.txt");
PARAM_INT("knns", "number of nearest neighbors to build the graph", "optfun",
  5);
PARAM_INT("leaf_size", "leaf_size for the tree.\
  if you choose the option with the nearest file you don't need to specify it",
  "optfun", 20);

//***UNDOCUMENTED VALUES***
PARAM(double, "desired_feasibility_error", "Undocumented value.", "optfun", 1,
  false);
PARAM(double, "grad_tolerance", "Undocumented value.", "optfun", 0.1, false);
PARAM(double, "infeasibility_tolerance", "Undocumented value.", "optfun", 0.01,
  false);


/*const fx_module_doc mvu_doc = {
  mvu_entries, NULL,
  " This program computes the Maximum Variance Unfolding"
  " and the Maximum Futhest Neighbor Unfolding as presented "
  " in the paper: \n"
  " @conference{vasiloglou2008ssm,\n"
  "   title={{Scalable semidefinite manifold learning}},\n"
  "   author={Vasiloglou, N. and Gray, A.G. and Anderson, D.V.},\n"
  "   booktitle={Machine Learning for Signal Processing, 2008. MLSP 2008. IEEE Workshop on},\n"
  "   pages={368--373},\n"
  "   year={2008}\n"
  " }\n"
};*/



class MaxVariance {
 public:
  static const size_t MAX_KNNS = 30;

  void Init(fx_module* module, arma::mat& data);
  void Init(fx_module* module);

  void Destruct();

  void ComputeGradient(const arma::mat& coordinates, arma::mat& gradient);
  void ComputeObjective(const arma::mat& coordinates, double& objective);
  void ComputeFeasibilityError(const arma::mat& coordinates, double& error);
  double ComputeLagrangian(const arma::mat& coordinates);
  void UpdateLagrangeMult(const arma::mat& coordinates);
  void Project(arma::mat& coordinates);
  void set_sigma(double sigma);
  bool IsDiverging(double objective);

  // what the hell is this? // I don't know, you tell me?
  bool IsOptimizationOver(const arma::mat& coordinates, arma::mat& gradient, double step) { return false; }
  bool IsIntermediateStepOver(const arma::mat& coordinates, arma::mat& gradient, double step) { return true; }

  void GiveInitMatrix(arma::mat& init_data);
  size_t num_of_points();

 private:
  datanode *module_;

  mlpack::allknn::AllkNN allknn_;
  size_t knns_;
  size_t leaf_size_;

  std::vector<std::pair<size_t, size_t> > nearest_neighbor_pairs_;
  std::vector<double> nearest_distances_;

  arma::vec eq_lagrange_mult_;

  size_t num_of_nearest_pairs_;
  double sigma_;
  double sum_of_furthest_distances_;
  size_t num_of_points_;
  size_t new_dimension_;
};

class MaxFurthestNeighbors {
 public:
  static const size_t MAX_KNNS = 30;

  void Init(fx_module *module, arma::mat& data);
  void Init(fx_module *module);

  void Destruct();

  void ComputeGradient(const arma::mat& coordinates, arma::mat& gradient);
  void ComputeObjective(const arma::mat& coordinates, double& objective);
  void ComputeFeasibilityError(const arma::mat& coordinates, double& error);
  double ComputeLagrangian(const arma::mat& coordinates);
  void UpdateLagrangeMult(const arma::mat& coordinates);
  void Project(arma::mat& coordinates);

  void set_sigma(double sigma); 
  void set_lagrange_mult(double val);

  bool IsDiverging(double objective); 
  bool IsOptimizationOver(arma::mat& coordinates, arma::mat& gradient, double step);
  bool IsIntermediateStepOver(arma::mat& coordinates, arma::mat& gradient, double step);

  size_t num_of_points();
  void GiveInitMatrix(arma::mat& init_data);

 private:
  datanode *module_;

  mlpack::allknn::AllkNN allknn_;
  AllkFN allkfn_;

  size_t knns_;
  size_t leaf_size_;

  std::vector<std::pair<size_t, size_t> > nearest_neighbor_pairs_;
  std::vector<double> nearest_distances_;

  arma::vec eq_lagrange_mult_;
  size_t num_of_nearest_pairs_;
  size_t num_of_furthest_pairs_;

  std::vector<std::pair<size_t, size_t> > furthest_neighbor_pairs_;
  std::vector<double> furthest_distances_;

  double sum_of_furthest_distances_;
  double sigma_;
  size_t num_of_points_;
  size_t new_dimension_;
  double infeasibility1_;
  double previous_infeasibility1_;
  double desired_feasibility_error_;
  double infeasibility_tolerance_;
  double sum_of_nearest_distances_;
  double grad_tolerance_;
};

class MaxVarianceUtils {
 public:
  static void ConsolidateNeighbors(const arma::Col<size_t>& from_tree_ind,
                                   const arma::vec&          from_tree_dist,
                                   size_t num_of_neighbors,
                                   size_t chosen_neighbors,
                                   std::vector<std::pair<size_t, size_t> >& neighbor_pairs,
                                   std::vector<double>& distances,
                                   size_t& num_of_pairs);

  static void EstimateKnns(const arma::Col<size_t>& nearest_neighbors,
                           const arma::vec&          nearest_distances,
                           size_t maximum_knns, 
                           size_t num_of_points,
                           size_t dimension,
                           size_t& optimum_knns); 
};

#include "mvu_objectives_impl.h"
#endif //MVU_OBJECTIVES_H_
