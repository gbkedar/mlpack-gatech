/*
 * =====================================================================================
 * 
 *       Filename:  mvu_dot_prod_objective.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  04/09/2008 05:30:43 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

#ifndef MVU_DOT_PROD_OBJECTIVE_H_
#define MVU_DOT_PROD_OBJECTIVE_H_
#include "fastlib/fastlib.h"
#include "../l_bfgs/optimization_utils.h"
class MVUDotProdObjective {
 public:
  void Init(datanode *module,
      Matrix *auxiliary_mat, 
      ArrayList<std::pair<size_t, size_t> > &pairs_to_consider, 
      // The values of the (row, column) values, also known as the dot products
      ArrayList<double> &dot_prod_values);
  void ComputeGradient(Matrix &coordinates, Matrix *gradient);
  void ComputeObjective(Matrix &coordinates, double *objective);
  void ComputeFeasibilityError(Matrix &coordinates, double *error);
  double ComputeLagrangian(Matrix &coordinates);
  void UpdateLagrangeMult(Matrix &coordinates);
  void Project(Matrix *coordinates);
  void set_sigma(double sigma); 
  bool IsDiverging(double objective); 

 private:
  datanode *module_;
  Matrix *auxiliary_mat_;
  ArrayList<std::pair<size_t, size_t> > pairs_to_consider_; 
  ArrayList<double> dot_prod_values_;
  Vector eq_lagrange_mult_;
  double sigma_;
  size_t num_of_constraints_;
};

class MVUDotProdObjectiveBounded {
 public:
  void Init(datanode *module,
      Matrix *auxiliary_mat, 
      ArrayList<std::pair<size_t, size_t> > &pairs_to_consider, 
      // The values of the (row, column) values, also known as the dot products
      ArrayList<double> &dot_prod_values);
  void ComputeGradient(Matrix &coordinates, Matrix *gradient);
  void ComputeObjective(Matrix &coordinates, double *objective);
  void ComputeFeasibilityError(Matrix &coordinates, double *error);
  double ComputeLagrangian(Matrix &coordinates);
  void UpdateLagrangeMult(Matrix &coordinates);
  void Project(Matrix *coordinates);
  void set_sigma(double sigma); 
  bool IsDiverging(double objective); 

 private:
  datanode *module_;
  Matrix *auxiliary_mat_;
  ArrayList<std::pair<size_t, size_t> > pairs_to_consider_; 
  ArrayList<double> dot_prod_values_;
  Vector ineq_low_lagrange_mult_;
  Vector ineq_high_lagrange_mult_;
  Vector low_bound_;
  Vector high_bound_;
  double sigma_;
  size_t num_of_constraints_;
};
#include "mvu_dot_prod_objective_impl.h"
#endif // MVU_DOT_PROD_OBJECTIVE_H_
