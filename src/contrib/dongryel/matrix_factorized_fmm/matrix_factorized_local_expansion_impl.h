#ifndef INSIDE_MATRIX_FACTORIZED_LOCAL_EXPANSCLIN_H
#error "This is not a public header file!"
#endif

#ifndef MATRIX_FACTORIZED_LOCAL_EXPANSCLIN_IMPL_H
#define MATRIX_FACTORIZED_LOCAL_EXPANSCLIN_IMPL_H

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::PrintDebug
(const char *name, FILE *stream) const {

}

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::CombineBasisFunctions
(MatrixFactorizedLocalExpansion &local_expansion1,
 MatrixFactorizedLocalExpansion &local_expansion2) {
  
  // The incoming skeleton for an internal node is formed by
  // concatenating the incoming skeleton of its children.
  const ArrayList<size_t> &incoming_skeleton1 =
    local_expansion1.incoming_skeleton();
  const ArrayList<size_t> &incoming_skeleton2 =
    local_expansion2.incoming_skeleton();

  incoming_skeleton_.Init(incoming_skeleton1.size() + 
			  incoming_skeleton2.size());
  for(size_t i = 0; i < incoming_skeleton1.size(); i++) {
    incoming_skeleton_[i] = incoming_skeleton1[i];
  }
  for(size_t i = incoming_skeleton1.size(); i < incoming_skeleton_.size();
      i++) {
    incoming_skeleton_[i] = incoming_skeleton2[i - incoming_skeleton1.size()];
  }

  // Allocate space for local moments based on the size of the
  // incoming skeleton.
  coeffs_.Init(incoming_skeleton_.size());
  coeffs_.SetZero();

  // Compute the beginning index and the count of the local expansion
  // for the children expansions.
  local_expansion1.set_local_to_local_translation_begin(0);
  local_expansion1.set_local_to_local_translation_count
    (incoming_skeleton1.size());
  local_expansion2.set_local_to_local_translation_begin
    (incoming_skeleton1.size());
  local_expansion2.set_local_to_local_translation_count
    (incoming_skeleton2.size());

  // Set the minimum estimated kernel sum to be the min of the two.
  estimated_min_kernel_sum_l_ = 
    std::min(local_expansion1.estimated_min_kernel_sum_l(),
	     local_expansion2.estimated_min_kernel_sum_l());
}

template<typename TKernelAux>
double MatrixFactorizedLocalExpansion<TKernelAux>::EvaluateField
(const Matrix& data, int row_num, int begin_row_num) const {
  
  // Take the dot product of the (row_num - begin_row_num) th row of
  // the evaluation operator.
  double dot_product = 0;

  for(size_t i = 0; i < evaluation_operator_->n_cols(); i++) {
    dot_product += 
      evaluation_operator_->get(row_num - begin_row_num, i) * coeffs_[i];
  }
  return dot_product;
}

template<typename TKernelAux>
double MatrixFactorizedLocalExpansion<TKernelAux>::EvaluateField
(const Vector& x_q) const {
  DEBUG_ASSERT_MSG(false, "Please implement me!");
  return -1;
}

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::Init
(const Vector& center, const TKernelAux &ka) {
  
  // Copy kernel type, center, and bandwidth squared
  kernel_ = &(ka.kernel_);
  ka_ = &ka;

  // Set the incoming representation to be null. This is only valid
  // for a leaf node.
  evaluation_operator_ = NULL;

  // Set the minimum kernel sum to be infinity.
  estimated_min_kernel_sum_l_ = DBL_MAX;
}

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::Init(const TKernelAux &ka) {
  
  // copy kernel type, center, and bandwidth squared
  kernel_ = &(ka.kernel_);
  ka_ = &ka;

  // Set the incoming representation to be null. This is only valid
  // for a leaf node.
  evaluation_operator_ = NULL;

  // Set the minimum kernel sum to be infinity.
  estimated_min_kernel_sum_l_ = DBL_MAX;
}

template<typename TKernelAux>
template<typename TBound>
int MatrixFactorizedLocalExpansion<TKernelAux>::OrderForEvaluating
(const TBound &far_field_region, 
 const TBound &local_field_region, double min_dist_sqd_regions,
 double max_dist_sqd_regions, double max_error, double *actual_error) const {
  
  return ka_->OrderForEvaluatingLocal(far_field_region, local_field_region, 
				     min_dist_sqd_regions,
				     max_dist_sqd_regions, max_error, 
				     actual_error);
}

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::SetZero() {

  // Clear the coefficients to zer.
  coeffs_.SetZero();
}

template<typename TKernelAux>
template<typename Tree>
void MatrixFactorizedLocalExpansion<TKernelAux>::TrainBasisFunctions
(const Matrix &query_set, int begin, int end, const Matrix *reference_set,
 const ArrayList<Tree *> *reference_leaf_nodes) {
  
  // The sample kernel matrix is |Q| by S where |Q| is the number of
  // query points in the query node and S is the number of reference
  // samples taken from the stratification.
  Matrix sample_kernel_matrix;
  int num_reference_samples = reference_leaf_nodes->size();
  int num_query_samples = end - begin;

  // Allocate a temporary space for holding the indices of the query
  // points, from which the incoming skeleton will be chosen.
  ArrayList<size_t> tmp_incoming_skeleton;
  tmp_incoming_skeleton.Init(num_query_samples);
  for(size_t q = 0; q < num_query_samples; q++) {

    // Choose a random query point and record its index.
    tmp_incoming_skeleton[q] = q + begin;
  }
  num_query_samples = tmp_incoming_skeleton.size();

  // After determining the number of query samples to take,
  // allocate the space for the sample kernel matrix to be computed.
  sample_kernel_matrix.Init(num_query_samples, num_reference_samples);
  
  for(size_t r = 0; r < reference_leaf_nodes->size(); r++) {

    // Choose a random reference point from the current reference strata...
    size_t random_reference_point_index =
      math::RandInt(((*reference_leaf_nodes)[r])->begin(),
		    ((*reference_leaf_nodes)[r])->end());
    const double *reference_point =
      reference_set->GetColumnPtr(random_reference_point_index);

    for(size_t c = 0; c < num_query_samples; c++) {
      
      // The current query point
      const double *query_point = 
	query_set.GetColumnPtr(tmp_incoming_skeleton[c]);	

      // Compute the pairwise distance and the kernel value.
      double squared_distance =
	la::DistanceSqEuclidean(query_set.n_rows(), reference_point,
				query_point);
      double kernel_value = (ka_->kernel_).EvalUnnormOnSq(squared_distance);
      sample_kernel_matrix.set(c, r, kernel_value *
			       (((*reference_leaf_nodes)[r])->count()));
      
    } // end of iterating over each sample query strata...
  } // end of iterating over each reference point...
  
  // Get the estimate on the minimum kernel sum for the query
  // points. This code is not currently correct for weighted kernel
  // sum.
  for(size_t c = 0; c < num_query_samples; c++) {
    double kernel_sum = 0;

    for(size_t r = 0; r < sample_kernel_matrix.n_cols(); r++) {
      kernel_sum += sample_kernel_matrix.get(c, r);
    }
    estimated_min_kernel_sum_l_ = std::min(estimated_min_kernel_sum_l_, 
					   kernel_sum);
  }

  // CUR-decompose the sample kernel matrix.
  Matrix c_mat, u_mat, r_mat;
  ArrayList<size_t> column_indices, row_indices;
  CURDecomposition::Compute(sample_kernel_matrix, &c_mat, &u_mat, &r_mat,
			    &column_indices, &row_indices);
  
  // The incoming skeleton is constructed from the sampled rows in the
  // matrix factorization.
  incoming_skeleton_.Init(row_indices.size());
  for(size_t s = 0; s < row_indices.size(); s++) {
    incoming_skeleton_[s] = tmp_incoming_skeleton[row_indices[s]];
  }

  // Compute the evaluation operator, which is the product of the C
  // and the U factor appropriately scaled by the row scaled R factor.
  evaluation_operator_ = new Matrix();
  la::MulInit(c_mat, u_mat, evaluation_operator_);
  for(size_t i = 0; i < r_mat.n_rows(); i++) {
    double scaling_factor =
      (sample_kernel_matrix.get(row_indices[i], 0) < DBL_EPSILON) ?
      0:r_mat.get(i, 0) / sample_kernel_matrix.get(row_indices[i], 0);

    for(size_t j = 0; j < evaluation_operator_->n_rows(); j++) {
      evaluation_operator_->set
	(j, i, evaluation_operator_->get(j, i) * scaling_factor);
    }
  }

  // Allocate space based on the size of the incoming skeleton.
  coeffs_.Init(incoming_skeleton_.size());
  coeffs_.SetZero();
}

template<typename TKernelAux>
void MatrixFactorizedLocalExpansion<TKernelAux>::TranslateToLocal
(MatrixFactorizedLocalExpansion &se) const {
  
  // Local-to-local translation involves determining the indices of
  // the query points that belong to the local moment to be
  // translated.
  size_t beginning_index = se.local_to_local_translation_begin();
  size_t count = se.local_to_local_translation_count();
  
  // Reference to the destination coefficients.
  Vector &destination_coeffs = se.coeffs();

  for(size_t i = 0; i < count; i++) {
    destination_coeffs[i] += coeffs_[i + beginning_index];
  }
}

#endif
