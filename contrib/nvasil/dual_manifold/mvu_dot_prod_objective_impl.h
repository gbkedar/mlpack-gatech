/*
 * =====================================================================================
 * 
 *       Filename:  mvu_dot_prod_objective_impl.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  04/09/2008 06:24:01 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

void MVUDotProdObjective::Init(datanode *module,
    Matrix *auxiliary_mat, 
    ArrayList<std::pair<size_t, size_t> > &pairs_to_consider, 
    // The values of the (row, column) values, also known as the dot products
    ArrayList<double> &dot_prod_values) {
  
  module_=module;
  auxiliary_mat_=auxiliary_mat;
  pairs_to_consider_.InitCopy(pairs_to_consider);
  dot_prod_values_.InitCopy(dot_prod_values);
  eq_lagrange_mult_.Init(dot_prod_values.size());
  eq_lagrange_mult_.SetAll(0.0);
  num_of_constraints_=dot_prod_values_.size();
}

void MVUDotProdObjective::ComputeGradient(Matrix &coordinates, Matrix *gradient) {
  // we need to use -CRR^T because we want to maximize CRR^T
  gradient->CopyValues(coordinates);
  la::Scale(-1.0, gradient);
  gradient->SetAll(0.0);
  size_t dimension=auxiliary_mat_->n_rows();
  for (size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    la::AddExpert(dimension,  
        -eq_lagrange_mult_[i]+sigma_*diff,
        p2,
        gradient->GetColumnPtr(ind1));
  } 
}

void MVUDotProdObjective::ComputeObjective(Matrix &coordinates, double *objective) {
  *objective=0;
  size_t dimension = coordinates.n_rows();
  for(size_t i=0; i< coordinates.n_cols(); i++) {
     *objective-=la::Dot(dimension, 
                        coordinates.GetColumnPtr(i),
                        coordinates.GetColumnPtr(i));
  }
  
}

void MVUDotProdObjective::ComputeFeasibilityError(Matrix &coordinates, double *error) {
  size_t dimension = coordinates.n_rows();
  *error=0;
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    *error +=fabs(diff);
  }
  *error/=pairs_to_consider_.size();
}

double MVUDotProdObjective::ComputeLagrangian(Matrix &coordinates) {
  double lagrangian=0;
  size_t dimension = coordinates.n_rows();
  ComputeObjective(coordinates, &lagrangian);
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    lagrangian+= -eq_lagrange_mult_[i]*diff + sigma_*diff*diff;
  }
  return lagrangian;
}

void MVUDotProdObjective::UpdateLagrangeMult(Matrix &coordinates) {
  size_t dimension=coordinates.n_rows();
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    eq_lagrange_mult_[i]-=sigma_*diff;
  }
}

void MVUDotProdObjective::Project(Matrix *coordinates) {
  OptUtils::NonNegativeProjection(coordinates);
  // OptUtils::BoundProjection(coordinates, 0, 1.0);
}

void MVUDotProdObjective::set_sigma(double sigma) {
   sigma_=sigma;
} 

bool MVUDotProdObjective::IsDiverging(double objective) {
  return false;
}

///////////////////////////////////////
///////////////////////////////////////

void MVUDotProdObjectiveBounded::Init(datanode *module,
    Matrix *auxiliary_mat, 
    ArrayList<std::pair<size_t, size_t> > &pairs_to_consider, 
    // The values of the (row, column) values, also known as the dot products
    ArrayList<double> &dot_prod_values) {
  
  module_=module;
  auxiliary_mat_=auxiliary_mat;
  pairs_to_consider_.InitCopy(pairs_to_consider);
  dot_prod_values_.InitCopy(dot_prod_values);
  ineq_low_lagrange_mult_.Init(dot_prod_values.size());
  ineq_low_lagrange_mult_.SetAll(1.0);
  ineq_high_lagrange_mult_.Init(dot_prod_values.size());
  ineq_high_lagrange_mult_.SetAll(1.0); 
  num_of_constraints_=dot_prod_values_.size();
}

void MVUDotProdObjectiveBounded::ComputeGradient(Matrix &coordinates, Matrix *gradient) {
  // we need to use -CRR^T because we want to maximize CRR^T
  gradient->CopyValues(coordinates);
  la::Scale(-1.0, gradient);
  gradient->SetAll(0.0);
  size_t dimension=auxiliary_mat_->n_rows();
  for (size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    la::AddExpert(dimension,  
        -eq_lagrange_mult_[i]+sigma_*diff,
        p2,
        gradient->GetColumnPtr(ind1));
  } 
}

void MVUDotProdObjectiveBounded::ComputeObjective(Matrix &coordinates, double *objective) {
  *objective=0;
  size_t dimension = coordinates.n_rows();
  for(size_t i=0; i< coordinates.n_cols(); i++) {
     *objective-=la::Dot(dimension, 
                        coordinates.GetColumnPtr(i),
                        coordinates.GetColumnPtr(i));
  }
  
}

void MVUDotProdObjectiveBounded::ComputeFeasibilityError(Matrix &coordinates, double *error) {
  size_t dimension = coordinates.n_rows();
  *error=0;
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    *error +=fabs(diff);
  }
  *error/=pairs_to_consider_.size();
}

double MVUDotProdObjectiveBounded::ComputeLagrangian(Matrix &coordinates) {
  double lagrangian=0;
  size_t dimension = coordinates.n_rows();
  ComputeObjective(coordinates, &lagrangian);
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    lagrangian+= -eq_lagrange_mult_[i]*diff + sigma_*diff*diff;
  }
  return lagrangian;
}

void MVUDotProdObjectiveBounded::UpdateLagrangeMult(Matrix &coordinates) {
  size_t dimension=coordinates.n_rows();
  for(size_t i=0; i<num_of_constraints_; i++) {
    size_t ind1=pairs_to_consider_[i].first; 
    size_t ind2=pairs_to_consider_[i].second;
    double *p1=coordinates.GetColumnPtr(ind1);
    double *p2=auxiliary_mat_->GetColumnPtr(ind2);
    double dot_prod =la::Dot(dimension, p1, p2);
    double diff=dot_prod-dot_prod_values_[i];
    eq_lagrange_mult_[i]-=sigma_*diff;
  }
}

void MVUDotProdObjectiveBounded::Project(Matrix *coordinates) {
  OptUtils::NonNegativeProjection(coordinates);
  // OptUtils::BoundProjection(coordinates, 0, 1.0);
}

void MVUDotProdObjectiveBounded::set_sigma(double sigma) {
   sigma_=sigma;
} 

bool MVUDotProdObjectiveBounded::IsDiverging(double objective) {
  return false;
}

