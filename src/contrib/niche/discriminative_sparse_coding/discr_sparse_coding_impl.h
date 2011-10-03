#ifndef INSIDE_DISCR_SPARSE_CODING_H
#error "This is not a public header file!"
#endif



void DiscrSparseCoding::Init(const mat& X, const vec& y, u32 n_atoms,
			     double lambda_1, double lambda_2,
			     double lambda_w) {
  X_ = X;
  y_ = y;
  
  n_dims_ = X.n_rows;
  n_points_ = X.n_cols;
  
  n_atoms_ = n_atoms;
  D_ = mat(n_dims_, n_atoms_);
  
  w_ = vec(n_atoms_);
  
  
  lambda_1_ = lambda_1;
  lambda_2_ = lambda_2;
  lambda_w_ = lambda_w;
}


void DiscrSparseCoding::SetDictionary(const mat& D) {
  D_ = D;
}


void DiscrSparseCoding::SetW(const vec& w) {
  w_ = w;
}


void DiscrSparseCoding::InitDictionary() {  
  RandomInitDictionary();
}


void DiscrSparseCoding::InitDictionary(const char* dictionary_filename) {  
  D_.load(dictionary_filename);
}


void DiscrSparseCoding::RandomInitDictionary() {
  for(u32 j = 0; j < n_atoms_; j++) {
    D_.col(j) = randu(n_dims_);
    D_.col(j) /= norm(D_.col(j), 2);
  }
}


void DiscrSparseCoding::KMeansInitDictionary() {
  // need a constrained k-means algorithm to ensure each cluster is assigned at least one point
}


void DiscrSparseCoding::InitW() {
  w_.zeros();
}


void DiscrSparseCoding::InitW(const char* w_filename) {  
  w_.load(w_filename);
}


// this only handles sparse coding, not local coordinate coding
void DiscrSparseCoding::SGDOptimize(u32 n_iterations) {
  //InitDictionary();
  //InitW();
  
  
  for(u32 t = 1; t <= n_iterations; t++) {
    u32 ind = rand() % n_points_;
    
    // modify step size in some way
    //step_size = 1.0 / ((double)t);
    double step_size = 2.0 / (((double)t) + 2.0);
    //step_size = 1.0 / (sqrt(lambda_w_) * ((double)t));
    //step_size = 1.0 / (lambda_w_ * ((double)t));

    if((t % 1000) == 0) {
      printf("iteration %d: drew point %d\n", t, ind);
    }
    //((vec)(X_.col(ind))).print("point");
    //printf("label = %f\n", y_(ind));
    //PrintDictionary();
    //PrintW();
    //SGDStep(X_.col(ind), y_(ind), step_size);
    SGDStep(X_.colptr(ind), y_(ind), step_size);
    //PrintDictionary();
    //PrintW();
  }
}


// this only handles sparse coding, not local coordinate coding
//void DiscrSparseCoding::SGDStep(const vec& x, double y, double step_size) {
void DiscrSparseCoding::SGDStep(double* x_mem, double y, double step_size) {
  vec x = vec(x_mem, n_dims_, false, true); 

  
  // THE LARS WAY
  //printf("sgdstep\n");
  Lars lars;
  lars.Init(D_.memptr(), x_mem, n_dims_, n_atoms_, true, 0.5 * lambda_1_, lambda_2_);
  lars.DoLARS();
  vec v;
  lars.Solution(v);
  

  /*
  // THE SPARSE CODING WAY
  SparseCoding sc;
  sc.Init(x_mem, n_dims_, 1, n_atoms_, lambda_1_);
  sc.SetDictionary(D_.memptr());
  sc.OptimizeCode();
  vec v;
  sc.GetCoding(v);
  
  Problem: We aren't able to access the delightful cholesky factor, unless we change the sparse coding code to store this.
           Hence, we compute the LARS way for now.
  */
  

  if(y * dot(v, w_) > 1) {
    // no update necessary
    return;
  }
  
  mat chol_factor;
  lars.GetCholFactor(chol_factor);
  //printf("chol_factor is %d by %d\n", chol_factor.n_rows, chol_factor.n_cols);
  
  
  // active set
  std::vector<u32> active_set = lars.active_set();
  u32 n_active = active_set.size();
  
  if(n_active == 0) {
    printf("empty active set!\n");
    // this case is indicative of a possibly poor dictionary, but computationally, it's fucking great!
    //   no update to w, since v is zero
    //   no update to D, since D_active is 0-dimensional
    return;    
  }
  
  /*
  printf("active set: ( ");
  for(u32 i = 0; i < n_active; i++) {
    printf("%d ", active_set[i]);
  }
  printf(")\n");
  */
  
  // for the update, we need (D_Lambda^T + D_Lambda)^{-1}
  // fortunately, we already have the cholesky factorization of this in LARS
  
  // first, set up some things that will be useful later
  vec w_active = vec(n_active);
  mat D_active = mat(n_dims_, n_active);
  vec v_active = vec(n_active);
  vec sign_v_active = vec(n_active);

  for(u32 i = 0; i < n_active; i++) {
    u32 ind = active_set[i];
    w_active(i) = w_(ind);
    D_active.col(i) = D_.col(ind);
    double cur_v = v(ind);
    v_active(i) = cur_v;
    sign_v_active(i) = cur_v / fabs(cur_v);
  }
  
  // first, update D_active
  
  // Let A := inv(D^T * D + lambda_2 * I)
  
  // 3 parts (there's a 4th part for discriminative LCC)
  // 1st part:
  //vec A_w = solve(chol_factor, solve(trans(chol_factor), w_active));
  vec A_w = solve(trimatu(chol_factor), solve_trans(trimatu(chol_factor), w_active));
  rowvec wt_A = trans(A_w);
  //wt_A.print("wt_A");
  //sign_v_active.print("sign_v_active");
  
  mat first_part_lhs = D_active;
  vec first_part_middle = A_w;
  rowvec first_part_rhs = trans(v_active);
  mat first_part = first_part_lhs * first_part_middle * first_part_rhs;

  // 2nd part:
  vec second_part_lhs = D_active * v_active;
  rowvec second_part_rhs = wt_A;
  mat second_part = second_part_lhs * second_part_rhs;

  // 3rd part:
  vec third_part_lhs = x;
  rowvec third_part_rhs = wt_A;
  mat third_part = -third_part_lhs * third_part_rhs;
  
  /*
  // the fourth part is only necessary for Discriminative LCC
  // 4th part:
  vec fourth_part_1_lhs = x;
  rowvec fourth_part_1_rhs = wt_A % trans(sign_v_active);
  mat fourth_part_2 = D_active;
  vec fourth_part_2_col_scaling = A_w % sign_v_active;
  mat fourth_part = 
    -fourth_part_1_lhs * fourth_part_1_rhs - 
    fourth_part_2 * diagmat(fourth_part_2_col_scaling);
  */

  // for Discriminative Sparse Coding
  mat D_active_update = step_size * y * (first_part + second_part + third_part);

  // for Discriminative LCC
  //mat D_active_update = step_size * y * (first_part + second_part + third_part + fourth_part);

  
  // now, update hypothesis vector w
  w_ -= step_size * (lambda_w_ * w_ - y * v);
  
  
  // now, update dictionary D using D_active
  // also, project each modified column onto unit ball
  for(u32 i = 0; i < n_active; i++) {
    u32 ind = active_set[i];
    D_.col(active_set[i]) -= D_active_update.col(i);
    
    // project active atoms onto unit ball
    double norm_D_ind = norm(D_.col(ind), 2);
    if(norm_D_ind > 1) {
      D_.col(ind) /= norm_D_ind;
    }
  }
  
  ProjectW();
}


void DiscrSparseCoding::ProjectW() {
  double norm_w = norm(w_, 2);
  // can we use 1 / lambda_w, as in Pegasos, or can we only use the weaker 2 / lambda_w?
  if(norm_w > sqrt(2.0 / lambda_w_)) {
    w_ = w_  * (sqrt((2.0 / lambda_w_)) / norm_w);
  }
}




void DiscrSparseCoding::PrintDictionary() {
  D_.print("Dictionary");
}

void DiscrSparseCoding::GetDictionary(mat& D) {
  D = D_;
}

void DiscrSparseCoding::PrintW() {
  w_.print("hypothesis vector w");
}

void DiscrSparseCoding::GetW(vec& w) {
  w = w_;
}
