/** @file krylov_lpr.h
 *
 *  This implementation can handle only non-negative training target
 *  values and points that lie the positive quadrant.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 *  @see krylov_lpr.cc
 *
 *  @bug No known bugs.
 */

#ifndef KRYLOV_LPR_H
#define KRYLOV_LPR_H

#include "fastlib/fastlib.h"
#include "epan_kernel_moment_info.h"
#include "multi_index_util.h"
#include "lpr_util.h"
#include "contrib/dongryel/proximity_project/gen_kdtree.h"
#include "mlpack/allknn/allknn.h"

#define INSIDE_KRYLOV_LPR_H
#include "krylov_stat.h"

/** @brief A computation class for dual-tree based local linear
 *         regression using a matrix-free Krylov subspace based
 *         method.
 */
template<typename TKernel, typename TPruneRule>
class KrylovLpr {

  FORBID_ACCIDENTAL_COPIES(KrylovLpr);

 public:

  ////////// Public Type Declarations //////////
  
  /** @brief The internal query tree type used for the computation. */
  typedef BinarySpaceTree< DHrectBound<2>, Matrix, 
			   KrylovLprQStat<TKernel> > QueryTree;

  /** @brief The internal reference tree type used for the
   *         computation.
   */
  typedef BinarySpaceTree< DHrectBound<2>, Matrix, 
			   KrylovLprRStat<TKernel> > ReferenceTree;

 private:

  ////////// Private Member Variables //////////

  /** @brief The required relative error. */
  double relative_error_;

  /** @brief The internal relative error tolerance required. */
  double internal_relative_error_;

  /** @brief The module holding the list of parameters. */
  struct datanode *module_;

  /** @brief The column-oriented reference dataset. */
  Matrix rset_;
 
  /** @brief The permutation mapping indices of references_ to
   *         original order.
   */
  ArrayList<size_t> old_from_new_references_;

  ArrayList<size_t> new_from_old_references_;

  /** @brief The reference tree. */
  ReferenceTree *rroot_;
  
  /** @brief The original training target value for the reference
   *         dataset.
   */
  Vector rset_targets_;

  /** @brief The reference training target value divided by the
   *         normalization constant of the kernel centered at each
   *         reference point.
   */
  Vector rset_target_divided_by_norm_consts_;

  /** @brief The inverse of the normalization constant of the kernel
   *         centered at each reference point.
   */
  Vector rset_inv_norm_consts_;

  /** @brief The inverses of the squared normalization constant of the
   *         kernel center at each reference point.
   */
  Vector rset_inv_squared_norm_consts_;

  /** @brief The original training target value for the reference
   *         dataset weighted by the reference coordinate.  (i.e. y_i
   *         [1; r^T]^T ).
   */
  Matrix target_weighted_rset_;

  /** @brief The computed fit values at each reference point.
   */
  Vector rset_regression_estimates_;
  
  Vector leave_one_out_rset_regression_estimates_;

  /** @brief The confidence band on the fit at each reference point.
   */
  ArrayList<DRange> rset_confidence_bands_;
  
  /** @brief The influence value at each reference point.
   */
  Vector rset_influence_values_;
  
  /** @brief The magnitude of the weight diagram vector at each
   *         reference point.
   */
  Vector rset_magnitude_weight_diagrams_;
  
  /** @brief The first degree of freedom, i.e. the sum of the
   *         influence value at each reference point.
   */
  double rset_first_degree_of_freedom_;
  
  /** @brief The second degree of freedom, i.e. the sum of the
   *         magnitudes of the weight diagram at each reference point.
   */
  double rset_second_degree_of_freedom_;
  
  /** @brief The variance of the reference set.
   */
  double rset_variance_;

  /** @brief The root mean square deviation of the reference set.
   */
  double root_mean_square_deviation_;

  /** @brief The dimensionality of each point.
   */
  int dimension_;

  /** @brief The local polynomial approximation order.
   */
  int lpr_order_;

  /** @brief The length of each column vector in local linear regression.
   */
  int row_length_;

  /** @brief The kernel function to use.
   */
  ArrayList<TKernel> kernels_;
 
  /** @brief The minimum normalization constant among the kernels
   *         centered at the reference points.
   */
  double min_norm_const_;

  /** @brief The z-score for the confidence band.
   */
  double z_score_;

  /** @brief Finite difference prune statistics.
   */
  int num_finite_difference_prunes_;
  
  /** @brief Series expansion based prune statistics.
   */
  int num_epanechnikov_prunes_;
  
  ////////// Private Member Functions //////////

  /** @brief This function tests the first phase computation (i.e.,
   *         the computation of B^T W(q) Y vectors for each query
   *         point). Add this function call right after the
   *         ComputeRightHandSides_ function call to test the
   *         correctness.
   */
  void TestDualtreeComputation_
  (const Matrix &qset, const ArrayList<bool> *query_in_cg_loop,
   const bool confidence_band_computation_phase,
   const Vector &reference_weights, size_t column_index, 
   const Matrix &approximated);

  /** @brief Initialize the bound statistics relevant to the right
   *         hand side computation.
   */
  void InitializeQueryTree_(QueryTree *qnode, const Matrix &qset,
			    const ArrayList<bool> *query_in_cg_loop);

  /** @brief The postprocessing function to finalize the computation
   *         of the right-hand sides of the linear system for each
   *         query point.
   */
  void FinalizeQueryTree_
  (QueryTree *qnode, const Matrix &qset, 
   const ArrayList<bool> *query_in_cg_loop, 
   const bool confidence_band_computation_phase, Matrix &right_hand_sides_l, 
   Matrix &right_hand_sides_e, Vector &right_hand_sides_used_error, 
   Vector &right_hand_sides_n_pruned, 
   Matrix *leave_one_out_right_hand_sides_e);

  /** @brief Preprocess the reference tree for bottom up statistics
   *         computation.
   */
  void InitializeReferenceStatistics_(ReferenceTree *rnode, int column_index,
				      const Vector &weights);

  void SolveLinearProblems_(QueryTree *qroot, const Matrix &qset, 
			    const Matrix &right_hand_sides_e,
			    Matrix *leave_one_out_right_hand_sides_e,
			    const Matrix &query_expansions,
			    Matrix &solution_vectors_e,
			    Matrix *leave_one_out_solution_vectors_e,
			    Matrix &query_expansion_solution_vectors_e);

  /** @brief The base-case exhaustive computation for dual-tree based
   *         computation of B^T W(q) Y.
   *
   *  @param qnode The query node.
   *  @param rnode The reference node.
   */
  void DualtreeWeightedVectorSumBase_
  (QueryTree *qnode, ReferenceTree *rnode, const Matrix &qset,
   const ArrayList<bool> *query_in_cg_loop, 
   const bool confidence_band_computation_phase,
   Matrix &right_hand_sides_l, Matrix &right_hand_sides_e, 
   Vector &right_hand_sides_used_error, Vector &right_hand_sides_n_pruned);

  /** @brief The canonical case for dual-tree based computation of B^T
   *         W(q) Y.
   *
   *  @param qnode The query node.
   *  @param rnode The reference node.
   */
  void DualtreeWeightedVectorSumCanonical_
    (QueryTree *qnode, ReferenceTree *rnode, const Matrix &qset,
     const ArrayList<bool> *query_in_cg_loop,
     Matrix &right_hand_sides_l, Matrix &right_hand_sides_e, 
     Vector &right_hand_sides_used_error, Vector &right_hand_sides_n_pruned);

  /** @brief Finalize the regression estimate for each query point by
   *         taking the dot-product between [1; q^T] and the final
   *         solution vector for (B^T W(q) B)^+ (B^T W(q) Y).
   */
  void FinalizeRegressionEstimates_
  (const Matrix &qset, const Matrix &query_expansions,
   const Matrix &solution_vectors_e,
   Matrix *leave_one_out_solution_vectors_e, 
   const Matrix &expansion_solution_vectors_e,
   const Matrix &linear_transformed_expansion_solution_vectors_e,
   Vector &regression_estimates, Vector *leave_one_out_regression_estimates,
   Vector *query_magnitude_weight_diagrams, Vector *query_influence_values) {

    // Loop over each query point and take the dot-product.
    for(size_t i = 0; i < qset.n_cols(); i++) {

      // Make aliases of the current query point associated solution
      // vector.
      const double *query_pt_solution = solution_vectors_e.GetColumnPtr(i);

      // Retrieve the expansion of the current query point.
      const double *query_point_expansion = query_expansions.GetColumnPtr(i);

      // Retrieve the solution vector associated with the linear
      // system: (B^T W(q) B)^{-1} t(q)
      const double *query_point_expansion_solution =
	expansion_solution_vectors_e.GetColumnPtr(i);

      // Retrieve the linear transformed solution vector asociated
      // with the current query: (B^T W(q)^2 B) (B^T W(q) B)^{-1}
      // t(q).
      const double *linear_transformed_query_point_expansion_solution =
	linear_transformed_expansion_solution_vectors_e.GetColumnPtr(i);

      // Take the dot product between the query point solution and the
      // query point expansion to get the regression estimate.
      regression_estimates[i] = la::Dot(row_length_, query_pt_solution,
					query_point_expansion);

      // Now take care of the leave-one-out estimate, if we have to.
      if(leave_one_out_solution_vectors_e != NULL) {
	const double *query_pt_leave_one_out_solution =
	  leave_one_out_solution_vectors_e->GetColumnPtr(i);
	(*leave_one_out_regression_estimates)[i] =
	  la::Dot(row_length_, query_pt_leave_one_out_solution,
		  query_point_expansion);
      }

      // Compute the magnitude of the weight diagram vector for each
      // query, i.e. sqrt(r(q)^T (B^T W(q) B)^-1 B^T W(q)^2 B (B^T
      // W(q) B)^-1 r(q)).
      if(query_magnitude_weight_diagrams != NULL) {
	(*query_magnitude_weight_diagrams)[i] =
	  sqrt(la::Dot(row_length_, query_point_expansion_solution,
		       linear_transformed_query_point_expansion_solution));
      }

      // Compute the influence value at each point (if it belongs to
      // the reference set), i.e. (r(q))^T (B^T W(q) B)^-1 B^T W(q)
      // e_i = (r(q))^T (B^T W(q) B)-1 W(0) r(q).
      if(query_influence_values != NULL) {
	(*query_influence_values)[i] =
	  la::Dot(row_length_, query_point_expansion_solution,
		  query_point_expansion) /
	  kernels_[i].CalcNormConstant(dimension_);
      }
    }
  }
  
  /** @brief Computes the root mean square deviation of the current
   *         model. This function should be called after the model has
   *         been completely built.
   */
  void ComputeRootMeanSquareDeviation_() {
    
    root_mean_square_deviation_ = 0;
    for(size_t i = 0; i < rset_.n_cols(); i++) {
      
      double diff_regression = rset_targets_[new_from_old_references_[i]] - 
	leave_one_out_rset_regression_estimates_[i];
      root_mean_square_deviation_ += diff_regression * diff_regression;
    }
    root_mean_square_deviation_ *= 1.0 / ((double) rset_.n_cols());
    root_mean_square_deviation_ = sqrt(root_mean_square_deviation_);
  }
  
  /** @brief Computes the variance by the normalized redisual sum of
   *         squares for the reference dataset.
   */
  void ComputeVariance_() {
    
    // Compute the degrees of freedom, i.e. the sum of the influence
    // values at each reference point and the sum of the squared
    // magnitudes of the weight diagram vectors at each reference
    // point.
    rset_first_degree_of_freedom_ = rset_second_degree_of_freedom_ = 0;
    for(size_t i = 0; i < rset_.n_cols(); i++) {
      rset_first_degree_of_freedom_ += rset_influence_values_[i];
      rset_second_degree_of_freedom_ += rset_magnitude_weight_diagrams_[i] * 
	rset_magnitude_weight_diagrams_[i];
    }

    // Reset the sum accumulated to zero.
    rset_variance_ = 0;
    
    // Loop over each reference point and add up the residual.
    for(size_t i = 0; i < rset_.n_cols(); i++) {
      double prediction_error = 
	rset_targets_[new_from_old_references_[i]] - 
	rset_regression_estimates_[i];
      rset_variance_ += prediction_error * prediction_error;
    }
    rset_variance_ *= 1.0 / 
      (rset_.n_cols() - 2.0 * rset_first_degree_of_freedom_ +
       rset_second_degree_of_freedom_);

    // This MIGHT happen if we have too few data points...
    if(rset_.n_cols() - 2.0 * rset_first_degree_of_freedom_ +
       rset_second_degree_of_freedom_ <= 0) {
      rset_variance_ = DBL_MAX;
    }

    fx_format_result(module_, 
		     "krylov_lpr_reference_set_first_degree_of_freedom",
		     "%g", rset_first_degree_of_freedom_);
    fx_format_result(module_, 
		     "krylov_lpr_reference_set_second_degree_of_freedom",
		     "%g", rset_second_degree_of_freedom_);
    fx_format_result(module_, "krylov_lpr_reference_set_variance", "%g",
		     rset_variance_);
  }
  
  void ComputeConfidenceBands_(const Matrix &queries,
			       Vector *query_regression_estimates,
			       ArrayList<DRange> *query_confidence_bands,
			       Vector *query_magnitude_weight_diagrams,
			       bool queries_equal_references) {
    
    // Initialize the storage for the confidene bands.
    query_confidence_bands->Init(queries.n_cols());
    
    for(size_t q = 0; q < queries.n_cols(); q++) {
      DRange &q_confidence_band = (*query_confidence_bands)[q];
      double spread;
      
      if(queries_equal_references) {
	spread = z_score_ * (*query_magnitude_weight_diagrams)[q] * 
	  sqrt(rset_variance_);
      }
      else {
	spread = z_score_ * (1 + (*query_magnitude_weight_diagrams)[q]) * 
	  sqrt(rset_variance_);	  
      }
      
      q_confidence_band.lo = (*query_regression_estimates)[q] - spread;
      q_confidence_band.hi = (*query_regression_estimates)[q] + spread;
    }
  }

  void DecideComputationMethod_
  (QueryTree *qnode, ReferenceTree *rnode, const Matrix &qset, 
   const ArrayList<bool> *query_in_cg_loop,
   const bool confidence_band_computation_phase,
   Matrix &right_hand_sides_l, Matrix &right_hand_sides_e, 
   Vector &right_hand_sides_used_error, Vector &right_hand_sides_n_pruned);

  void StratifiedComputation_
  (QueryTree *qroot, const Matrix &qset,
   const ArrayList<bool> *query_in_cg_loop, 
   const bool confidence_band_computation_phase, Matrix &right_hand_sides_l,
   Matrix &right_hand_sides_e, Vector &right_hand_sides_used_error,
   Vector &right_hand_sides_n_pruned);

  /** @brief Compute B^T W(q) Y vector for each query point, which
   *         essentially becomes the right-hand side for the linear
   *         system associated with each query point: (B^T W(q) B)
   *         z(q) = B^T W(q) Y. This function calls a dual-tree based
   *         fast vector summation to achieve this effect.
   */
  void ComputeWeightedVectorSum_
    (QueryTree *qroot, const Matrix &qset, const Vector &weights,
     const ArrayList<bool> *query_in_cg_loop, 
     const bool confidence_band_computation_phase, size_t column_index, 
     Matrix &right_hand_sides_l, Matrix &right_hand_sides_e, 
     Vector &right_hand_sides_used_error, Vector &right_hand_sides_n_pruned,
     Matrix *leave_one_out_right_hand_sides_e) {

    // Initialize the weight statistics on the reference side.
    InitializeReferenceStatistics_(rroot_, column_index, weights);
	
    // Initialize the bound quantities on the query side.
    right_hand_sides_l.SetZero();
    right_hand_sides_e.SetZero();
    right_hand_sides_used_error.SetZero();
    right_hand_sides_n_pruned.SetZero();
    if(leave_one_out_right_hand_sides_e != NULL) {
      leave_one_out_right_hand_sides_e->SetZero();
    }

    // Initialize the query tree summary statistics bound.
    InitializeQueryTree_(qroot, qset, query_in_cg_loop);
    
    // Call dualtree function.
    if(query_in_cg_loop == NULL && !confidence_band_computation_phase) {
      DualtreeWeightedVectorSumCanonical_
	(qroot, rroot_, qset, query_in_cg_loop, right_hand_sides_l, 
	 right_hand_sides_e, right_hand_sides_used_error, 
	 right_hand_sides_n_pruned);
    }
    else {
      StratifiedComputation_
	(qroot, qset, query_in_cg_loop, confidence_band_computation_phase,
	 right_hand_sides_l, right_hand_sides_e,
	 right_hand_sides_used_error, right_hand_sides_n_pruned);
    }

    // Final traversal of the query tree to finalize estimates.
    FinalizeQueryTree_(qroot, qset, query_in_cg_loop, 
		       confidence_band_computation_phase, right_hand_sides_l, 
		       right_hand_sides_e, right_hand_sides_used_error, 
		       right_hand_sides_n_pruned, 
		       leave_one_out_right_hand_sides_e);
  }

  void BasicComputeDualTree_
  (const Matrix &queries, Vector *query_regression_estimates,
   Vector *leave_one_out_query_regression_estimates,
   Vector *query_magnitude_weight_diagrams, Vector *query_influence_values) {
    
    // Zero out statistics.
    num_finite_difference_prunes_ = num_epanechnikov_prunes_ = 0;
    
    // Set relative error.
    relative_error_ = fx_param_double(module_, "relative_error", 0);
    internal_relative_error_ = relative_error_ / (relative_error_ + 2.0);

    // Copy the query set.
    Matrix qset;
    qset.Copy(queries);

    // read in the number of points owned by a leaf
    int leaflen = fx_param_int(module_, "leaflen", 40);
    
    // Construct the query tree.
    ArrayList<size_t> old_from_new_queries;
    QueryTree *qroot = tree::MakeKdTreeMidpoint<QueryTree>
      (qset, leaflen, &old_from_new_queries, NULL);

    // Initialize storage space for intermediate computations.
    Matrix right_hand_sides_l, right_hand_sides_e, solution_vectors_e;
    Matrix *leave_one_out_right_hand_sides_e = NULL;
    Matrix *leave_one_out_solution_vectors_e = NULL;
    Vector right_hand_sides_used_error, right_hand_sides_n_pruned;
    right_hand_sides_l.Init(row_length_, qset.n_cols());
    right_hand_sides_e.Init(row_length_, qset.n_cols());
    right_hand_sides_used_error.Init(qset.n_cols());
    right_hand_sides_n_pruned.Init(qset.n_cols());

    if(query_influence_values != NULL) {
      leave_one_out_right_hand_sides_e = new Matrix();
      leave_one_out_right_hand_sides_e->Init(row_length_, qset.n_cols());
      leave_one_out_solution_vectors_e = new Matrix();
      leave_one_out_solution_vectors_e->Init(row_length_, qset.n_cols());
    }
    solution_vectors_e.Init(row_length_, qset.n_cols());

    // The computation proceeds in three phases:
    //
    // Phase 1: Compute B^T W(q) Y vector for each query point.
    // Phase 2: Compute z(q) = (B^T W(q) B)^+ (B^T W(q) Y) for each query point
    //          using a matrix-free Krylov solver.
    // Phase 3: Compute [1; q^T] z(q) for each query point (the final
    //          post-processing step.)

    // The first phase computes B^T W(q) Y vector for each query
    // point. This essentially becomes the right-hand side for each
    // query point.
    printf("Starting Phase 1...\n");
    ComputeWeightedVectorSum_
      (qroot, qset, rset_target_divided_by_norm_consts_, NULL, false, 0,
       right_hand_sides_l, right_hand_sides_e, right_hand_sides_used_error, 
       right_hand_sides_n_pruned, leave_one_out_right_hand_sides_e);

    // Uncomment the following three lines to test the correctness of
    // the weighted vector sum.
    /*
    TestDualtreeComputation_(qset, NULL, false,
			     rset_target_divided_by_norm_consts_, 0,
			     right_hand_sides_e);
    */

    printf("Phase 1 completed...\n");

    // The second phase solves the least squares problem: (B^T W(q) B)
    // z(q) = B^T W(q) Y for each query point q.
    printf("Starting Phase 2...\n");
    Matrix query_expansions, query_expansion_solution_vectors_e;
    query_expansions.Init(row_length_, qset.n_cols());
    query_expansion_solution_vectors_e.Init(row_length_, qset.n_cols());
    MultiIndexUtil::ComputePointMultivariatePolynomial(dimension_, lpr_order_,
						       qset, query_expansions);
    SolveLinearProblems_(qroot, qset, right_hand_sides_e,
			 leave_one_out_right_hand_sides_e, 
			 query_expansions, solution_vectors_e,
			 leave_one_out_solution_vectors_e,
			 query_expansion_solution_vectors_e);

    // Now take the least squares solution to the system (B^T W(q)
    // B)^{-1} t(q) and multiple by the B^T W(q)^2 B for the final
    // pass.
    Matrix linear_transformed_query_expansion_solution_vectors_e;
    linear_transformed_query_expansion_solution_vectors_e.Init
      (row_length_, qset.n_cols());
    LinearOperatorConfidenceBand
      (qroot, qset, query_expansion_solution_vectors_e,
       linear_transformed_query_expansion_solution_vectors_e);

    // Free the query tree (very important!)
    delete qroot;
    printf("Phase 2 completed...\n");

    // Proceed with the third phase of the computation to output the
    // final regression value.
    printf("Starting Phase 3...\n");
    query_regression_estimates->Init(qset.n_cols());
    if(leave_one_out_query_regression_estimates != NULL) {
      leave_one_out_query_regression_estimates->Init(qset.n_cols());
    }
    query_magnitude_weight_diagrams->Init(qset.n_cols());
    if(query_influence_values != NULL) {
      query_influence_values->Init(qset.n_cols());
    }
    FinalizeRegressionEstimates_
      (qset, query_expansions, solution_vectors_e, 
       leave_one_out_solution_vectors_e, query_expansion_solution_vectors_e,
       linear_transformed_query_expansion_solution_vectors_e,
       (*query_regression_estimates), leave_one_out_query_regression_estimates,
       query_magnitude_weight_diagrams, query_influence_values);
    printf("Phase 3 completed...\n");

    // Reshuffle the results to account for dataset reshuffling
    // resulted from tree constructions
    Vector tmp_q_results;
    tmp_q_results.Init(query_regression_estimates->length());    
    for(size_t i = 0; i < tmp_q_results.length(); i++) {
      tmp_q_results[old_from_new_queries[i]] =	
	(*query_regression_estimates)[i];
    }
    query_regression_estimates->CopyValues(tmp_q_results);

    if(leave_one_out_query_regression_estimates != NULL) {
      for(size_t i = 0; i < tmp_q_results.length(); i++) {
	tmp_q_results[old_from_new_queries[i]] =	
	  (*leave_one_out_query_regression_estimates)[i];
      }
      leave_one_out_query_regression_estimates->CopyValues(tmp_q_results);
    }
    if(query_magnitude_weight_diagrams != NULL) {
      for(size_t i = 0; i < tmp_q_results.length(); i++) {
	tmp_q_results[old_from_new_queries[i]] =	
	  (*query_magnitude_weight_diagrams)[i];
      }
      query_magnitude_weight_diagrams->CopyValues(tmp_q_results);
    }
    if(query_influence_values != NULL) {
      for(size_t i = 0; i < tmp_q_results.length(); i++) {
	tmp_q_results[old_from_new_queries[i]] =
	  (*query_influence_values)[i];
      }
      query_influence_values->CopyValues(tmp_q_results);
    }

    // Memory cleanup
    if(leave_one_out_right_hand_sides_e != NULL) {
      delete leave_one_out_right_hand_sides_e;
      delete leave_one_out_solution_vectors_e;
    }
  }

  void ComputeMain_(const Matrix &queries,
		    Vector *query_regression_estimates,
		    Vector *leave_one_out_query_regression_estimates,
		    ArrayList<DRange> *query_confidence_bands,
		    Vector *query_magnitude_weight_diagrams,
		    Vector *query_influence_values) {
    
    // Clear prune statistics.
    num_finite_difference_prunes_ = num_epanechnikov_prunes_ = 0;
    
    // This is the basic N-body based computation.
    BasicComputeDualTree_(queries, query_regression_estimates,
			  leave_one_out_query_regression_estimates,
			  query_magnitude_weight_diagrams,
			  query_influence_values);
    
    printf("Number of finite difference prunes: %d\n",
	   num_finite_difference_prunes_);
    printf("Number of Epanechnikov prunes: %d\n", num_epanechnikov_prunes_);
    
    // If the reference dataset is being used for training, then
    // compute variance and degrees of freedom.
    if(query_influence_values != NULL) {
      ComputeVariance_();
    }
    
    // Compute the confidence band around each query point.
    ComputeConfidenceBands_(queries, query_regression_estimates,
			    query_confidence_bands,
			    query_magnitude_weight_diagrams,
			    (query_influence_values != NULL));
        
    // If the reference dataset is being used for training, then
    // compute the root mean square deviation.
    if(query_influence_values != NULL) {
      ComputeRootMeanSquareDeviation_();
    }
  }

  /** @brief Initialize the bandwidth by either fixed bandwidth
   *         parameter or a nearest neighbor based one (i.e. perform
   *         nearest neighbor and set the bandwidth equal to the k-th
   *         nearest neighbor distance).
   */
  void InitializeBandwidths_() {

    kernels_.Init(rset_.n_cols());

    if(fx_param_exists(NULL, "bandwidth")) {
      printf("Using the fixed bandwidth method...\n");

      double bandwidth = fx_param_double_req(NULL, "bandwidth");
      for(size_t i = 0; i < kernels_.size(); i++) {	
	kernels_[i].Init(bandwidth);
      }
    }
    else {
      printf("Using the nearest neighbor method...\n");
      AllkNN all_knn;
      double knn_factor = fx_param_double(NULL, "knn_factor", 0.001);
      int knns = (int) (knn_factor * rset_.n_cols());

      printf("Each reference point will look for %d nearest neighbors...\n",
	     knns);

      all_knn.Init(rset_, 20, knns);
      ArrayList<size_t> resulting_neighbors;
      ArrayList<double> distances;
      
      all_knn.ComputeNeighbors(&resulting_neighbors, &distances);

      for(size_t i = 0; i < distances.size(); i += knns) {
	kernels_[i / knns].Init(sqrt(distances[i + knns - 1]));
      }
    }
  }

  void PrecomputeWeights_() {
    rset_target_divided_by_norm_consts_.Init(rset_.n_cols());
    rset_inv_norm_consts_.Init(rset_.n_cols());
    rset_inv_squared_norm_consts_.Init(rset_.n_cols());
    
    for(size_t i = 0; i < rset_.n_cols(); i++) {
      rset_target_divided_by_norm_consts_[i] = 
	rset_targets_[i] / kernels_[i].CalcNormConstant(dimension_);
      rset_inv_norm_consts_[i] = 
	1.0 / kernels_[i].CalcNormConstant(dimension_);
      rset_inv_squared_norm_consts_[i] = 1.0 /
	(kernels_[i].CalcNormConstant(dimension_) *
	 kernels_[i].CalcNormConstant(dimension_));
    }
  }

  void LinearOperatorConfidenceBand
  (QueryTree *qroot, const Matrix &qset, 
   const Matrix &query_expansion_solution_vectors,
   Matrix &linear_transformed_query_expansion_solution_vectors);

 public:
  
  ////////// Constructor/Destructor //////////
  
  /** @brief The constructor that sets every pointer owned by this
   *         object to NULL.
   */
  KrylovLpr() {
    rroot_ = NULL;
  }

  /** @brief The destructor that frees memory owned by the trees.
   */
  ~KrylovLpr() {
    delete rroot_;
  }

  ////////// Getter/Setters //////////

  /** @brief Get the regression estimates.
   *
   *  @param results The uninitialized vector which will be filled
   *                 with the computed regression estimates.
   */
  void get_regression_estimates(Vector *results) { 
    results->Init(rset_regression_estimates_.length());
    
    for(size_t i = 0; i < rset_regression_estimates_.length(); i++) {
      (*results)[i] = rset_regression_estimates_[i];
    }
  }

  /** @brief Gets the confidence bands of the model.
   */
  void get_confidence_bands
  (ArrayList<DRange> *rset_confidence_bands_copy) {
    
    rset_confidence_bands_copy->Copy(rset_confidence_bands_);
  }

  double root_mean_square_deviation() {
    return root_mean_square_deviation_;
  }

  ////////// User-level Functions //////////

  void LinearOperator
  (QueryTree *qroot, const Matrix &qset,
   const ArrayList<bool> &query_in_cg_loop, const Matrix &original_vectors,
   Matrix *loo_original_vectors, const Matrix &expansion_original_vectors,
   Matrix &linear_transformed_vectors, Matrix *linear_transformed_loo_vectors,
   Matrix &linear_transformed_expansion_vectors);

  /** @brief Computes the query regression estimates with the
   *         confidence bands.
   */
  void Compute(const Matrix &queries, Vector *query_regression_estimates,
	       ArrayList<DRange> *query_confidence_bands,
	       Vector *query_magnitude_weight_diagrams) {
    
    fx_timer_start(module_, "krylov_lpr_prediction_time");
    ComputeMain_(queries, query_regression_estimates, NULL, 
		 query_confidence_bands, query_magnitude_weight_diagrams, 
		 NULL);
    fx_timer_stop(module_, "krylov_lpr_prediction_time");
  }

  void Init(Matrix &references, Matrix &reference_targets,
	    struct datanode *module_in) {
    
    // set the incoming parameter module.
    module_ = module_in;
    
    // read in the number of points owned by a leaf
    int leaflen = fx_param_int(module_in, "leaflen", 40);
    
    // set the local polynomial approximation order.
    lpr_order_ = fx_param_int_req(NULL, "lpr_order");
    
    // copy reference dataset and reference weights.
    rset_.Copy(references);
    rset_targets_.Copy(reference_targets.GetColumnPtr(0),
		       reference_targets.n_cols());
    
    // Record dimensionality and the appropriately cache the number of
    // components required.
    dimension_ = rset_.n_rows();
    row_length_ = (int) math::BinomialCoefficient(dimension_ + lpr_order_,
						  dimension_);
    
    // Set the z-score necessary for computing the confidence band.
    z_score_ = fx_param_double(module_, "z_score", 1.96);
    
    // Start measuring the tree construction time.
    fx_timer_start(NULL, "krylov_lpr_reference_tree_construct");
    
    // Construct the reference tree.
    rroot_ = tree::MakeKdTreeMidpoint<ReferenceTree>
      (rset_, leaflen, &old_from_new_references_, 
       &new_from_old_references_);
    
    // We need to shuffle the reference training target values
    // according to the shuffled order of the reference dataset.
    Vector tmp_rset_targets;
    tmp_rset_targets.Init(rset_targets_.length());
    for(size_t j = 0; j < rset_targets_.length(); j++) {
      tmp_rset_targets[j] = rset_targets_[old_from_new_references_[j]];
    }
    rset_targets_.CopyValues(tmp_rset_targets);
    fx_timer_stop(NULL, "krylov_lpr_reference_tree_construct");
    
    // Initialize the kernels.
    InitializeBandwidths_();
    PrecomputeWeights_();
        
    // Train the model using the reference set (i.e. compute
    // confidence interval and degrees of freedom.)
    fx_timer_start(module_, "krylov_lpr_training_time");

    // initialize the reference side statistics.
    target_weighted_rset_.Init(row_length_, rset_.n_cols());

    ComputeMain_(references, &rset_regression_estimates_,
		 &leave_one_out_rset_regression_estimates_,
		 &rset_confidence_bands_, &rset_magnitude_weight_diagrams_,
		 &rset_influence_values_);
    fx_timer_stop(module_, "krylov_lpr_training_time");
  }

  void PrintDebug() {
    
    FILE *stream = stdout;
    const char *fname = NULL;
    
    if((fname = fx_param_str(module_, "fast_lpr_output", 
			     "fast_lpr_output.txt")) != NULL) {
      stream = fopen(fname, "w+");
    }
    for(size_t q = 0; q < rset_.n_cols(); q++) {
      fprintf(stream, "%g %g %g %g %g %g\n", rset_confidence_bands_[q].lo,
	      rset_regression_estimates_[q], rset_confidence_bands_[q].hi,
	      leave_one_out_rset_regression_estimates_[q],
	      rset_magnitude_weight_diagrams_[q],
	      rset_influence_values_[q]);
    }
    
    if(stream != stdout) {
      fclose(stream);
    }
  }

};

#include "krylov_lpr_setup_impl.h"
#include "krylov_lpr_solver_impl.h"
#include "krylov_lpr_test.h"
#undef INSIDE_KRYLOV_LPR_H

#endif
