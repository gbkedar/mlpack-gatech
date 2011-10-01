/*
 * =====================================================================================
 * 
 *       Filename:  relaxed_nmf.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  08/16/2008 12:11:24 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */
#ifndef RELAXED_NMF_BOUND_TIGHTENER_H_
#define RELAXED_NMF_BOUND_TIGHTENER_H_
#include "fastlib/fastlib.h"
#include "mlpack/allknn/allknn.h"

class RelaxedNmfBoundTightener {
 public:
  void Init(fx_module *module,
                      ArrayList<size_t> &rows,
                      ArrayList<size_t> &columns,
                      ArrayList<double> &values,
                      Matrix &x_lower_bound, 
                      Matrix &x_upper_bound,  
                      size_t opt_var_row,
                      size_t opt_var_column,
                      size_t opt_var_sign,
                      double function_upper_bound);
  void Destruct();
  void SetOptVarRowColumn(size_t row, size_t column);
  void SetOptVarSign(double sign);
  // The following are required by LBFGS
  void ComputeGradient(Matrix &coordinates, Matrix *gradient);
  void ComputeObjective(Matrix &coordinates, double *objective);
  // This class implements a convex relaxation of the nmf objective
  // At some point we need to compute the original objective the non relaxed
  void ComputeNonRelaxedObjective(Matrix &coordinates, double *objective);
  void ComputeFeasibilityError(Matrix &coordinates, double *error);
  double ComputeLagrangian(Matrix &coordinates);
  void UpdateLagrangeMult(Matrix &coordinates);
  void Project(Matrix *coordinates);
  void set_sigma(double sigma); 
  void GiveInitMatrix(Matrix *init_data);
	bool IsDiverging(double objective); 
  bool IsOptimizationOver(Matrix &coordinates, Matrix &gradient, double step);
  bool IsIntermediateStepOver(Matrix &coordinates, Matrix &gradient, double step);
  
  // The following are required by the branch and bound
  double GetSoftLowerBound();
    
 private:
  // holds all the info
  fx_module *module_;
  // number of rows of the original matrix
  size_t num_of_rows_;
  // number of columns of the original matrix
  size_t num_of_columns_;
  // offset of the H matrix on the coordinate variable
  size_t h_offset_;
  size_t w_offset_;
  double values_sq_norm_;
  size_t new_dimension_;
  // constant term for the LP relaxation part
  Vector a_linear_term_;
  // linear term for the LP relaxation part
  Vector b_linear_term_;
  ArrayList<size_t> rows_;
  ArrayList<size_t> columns_;
  ArrayList<double> values_;
  size_t opt_var_row_;
  size_t opt_var_column_;
  double opt_var_sign_;
  double function_upper_bound_;
  // lower bound for the optimization variable
  Matrix x_lower_bound_;
  // upper bound for the optimization variable
  Matrix x_upper_bound_;
  // soft lower bound of the relaxation
  double soft_lower_bound_;
  // tolerance for the gradient norm
  double grad_tolerance_;
  double sigma_;
  double desired_duality_gap_;
};

class RelaxedNmfIsometricBoundTightener {
 public:
  void Init(fx_module *module,
                      ArrayList<size_t> &rows,
                      ArrayList<size_t> &columns,
                      ArrayList<double> &values,
                      Matrix &x_lower_bound, 
                      Matrix &x_upper_bound,  
                      size_t opt_var_row,
                      size_t opt_var_column,
                      size_t opt_var_sign,
                      double function_upper_bound);
  void Destruct();
  void SetOptVarRowColumn(size_t row, size_t column);
  void SetOptVarSign(double sign);
  // The following are required by LBFGS
  void ComputeGradient(Matrix &coordinates, Matrix *gradient);
  void ComputeObjective(Matrix &coordinates, double *objective);
  // This class implements a convex relaxation of the nmf objective
  // At some point we need to compute the original objective the non relaxed
  void ComputeNonRelaxedObjective(Matrix &coordinates, double *objective);
  void ComputeFeasibilityError(Matrix &coordinates, double *error);
  double ComputeLagrangian(Matrix &coordinates);
  void UpdateLagrangeMult(Matrix &coordinates);
  void Project(Matrix *coordinates);
  void set_sigma(double sigma); 
  void GiveInitMatrix(Matrix *init_data);
	bool IsDiverging(double objective); 
  bool IsOptimizationOver(Matrix &coordinates, Matrix &gradient, double step);
  bool IsIntermediateStepOver(Matrix &coordinates, Matrix &gradient, double step);
   
  // The following are required by the branch and bound
  double GetSoftLowerBound();
  bool IsInfeasible();
    
 private:
  // holds all the info
  fx_module *module_;
  // number of rows of the original matrix
  size_t num_of_rows_;
  // number of columns of the original matrix
  size_t num_of_columns_;
  // offset of the H matrix on the coordinate variable
  size_t h_offset_;
  size_t w_offset_;
  double values_sq_norm_;
  size_t new_dimension_;
  double desired_duality_gap_;
  ArrayList<std::pair<size_t, size_t> > nearest_neighbor_pairs_;
  ArrayList<double> nearest_distances_;
  size_t num_of_nearest_pairs_;
  // constant term for the LP relaxation part of the objective
  Vector objective_a_linear_term_;
  // linear term for the LP relaxation part of the objective
  Vector objective_b_linear_term_;
  // constant term for the LP relaxation part of the constraints
  Vector constraint_a_linear_term_;
  // linear term for the LP relaxation part of the constraints
  Vector constraint_b_linear_term_;
  AllkNN allknn_;
  bool is_infeasible_;
  
  ArrayList<size_t> rows_;
  ArrayList<size_t> columns_;
  ArrayList<double> values_;
  size_t opt_var_row_;
  size_t opt_var_column_;
  double opt_var_sign_;
  double function_upper_bound_;
  // lower bound for the optimization variable
  Matrix x_lower_bound_;
  // upper bound for the optimization variable
  Matrix x_upper_bound_;
  // soft lower bound of the relaxation
  double soft_lower_bound_;
  // tolerance for the gradient norm
  double grad_tolerance_;
  double sigma_;
};

class RelaxedNmfIsometricBoxTightener {
 public:
  void Init(fx_module *module,
                      ArrayList<size_t> &rows,
                      ArrayList<size_t> &columns,
                      ArrayList<double> &values,
                      Vector &x_lower_bound, 
                      Vector &x_upper_bound,  
                      size_t opt_var_sign,
                      double function_upper_bound);
  void Destruct();
  void SetOptVarSign(double sign);
  // The following are required by LBFGS
  void ComputeGradient(Matrix &coordinates, Matrix *gradient);
  void ComputeObjective(Matrix &coordinates, double *objective);
  // This class implements a convex relaxation of the nmf objective
  // At some point we need to compute the original objective the non relaxed
  void ComputeNonRelaxedObjective(Matrix &coordinates, double *objective);
  void ComputeFeasibilityError(Matrix &coordinates, double *error);
  double ComputeLagrangian(Matrix &coordinates);
  void UpdateLagrangeMult(Matrix &coordinates);
  void Project(Matrix *coordinates);
  void set_sigma(double sigma); 
  void GiveInitMatrix(Matrix *init_data);
	bool IsDiverging(double objective); 
  bool IsOptimizationOver(Matrix &coordinates, Matrix &gradient, double step);
  bool IsIntermediateStepOver(Matrix &coordinates, Matrix &gradient, double step);
   
  // The following are required by the branch and bound
  double GetSoftLowerBound();
  bool IsInfeasible();
    
 private:
  // holds all the info
  fx_module *module_;
  // number of rows of the original matrix
  size_t num_of_rows_;
  // number of columns of the original matrix
  size_t num_of_columns_;
  // offset of the H matrix on the coordinate variable
  size_t h_offset_;
  size_t w_offset_;
  double values_sq_norm_;
  size_t new_dimension_;
  double desired_duality_gap_;
  ArrayList<std::pair<size_t, size_t> > nearest_neighbor_pairs_;
  ArrayList<double> nearest_distances_;
  size_t num_of_nearest_pairs_;
  // constant term for the LP relaxation part of the objective
  Vector objective_a_linear_term_;
  // linear term for the LP relaxation part of the objective
  Vector objective_b_linear_term_;
  // constant term for the LP relaxation part of the constraints
  Vector constraint_a_linear_term_;
  // linear term for the LP relaxation part of the constraints
  Vector constraint_b_linear_term_;
  AllkNN allknn_;
  bool is_infeasible_;
  
  ArrayList<size_t> rows_;
  ArrayList<size_t> columns_;
  ArrayList<double> values_;
  double opt_var_sign_;
  double function_upper_bound_;
  // lower bound for the optimization variable
  Vector x_lower_bound_;
  // upper bound for the optimization variable
  Vector x_upper_bound_;
  // soft lower bound of the relaxation
  double soft_lower_bound_;
  // tolerance for the gradient norm
  double grad_tolerance_;
  double sigma_;
};

#include "relaxed_nmf_bound_tightener_impl.h"
#endif
