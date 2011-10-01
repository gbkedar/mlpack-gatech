/*
 * =====================================================================================
 * 
 *       Filename:  gop_tight_nmf_engine_impl.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  08/19/2008 04:38:54 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

#ifdef GOP_TIGHT_NMF_ENGINE_H_
#ifndef GOP_TIGHT_NMF_ENGINE_IMPL_H_
#define GOP_TIGHT_NMF_ENGINE_IMPL_H_

void  GopTightNmfEngine::Init(fx_module *module, Matrix &data_matrix) {
  module_=module;
  // get the modules
  // module for the box tightener
  fx_module *relaxed_nmf_box_tightener_module=fx_submodule(module_, 
      "relaxed_nmf_tightener");
  // module for the branch and box optimization
  fx_module *gop_nmf_engine_module=fx_submodule(module_,"gop_nmf_engine");
  // module for the l_bfgs, we will need that for the tightenng
  // and for getting a universal upper_box
  fx_module *l_bfgs_module=fx_submodule(module_, "l_bfgs");
  // module for the relaxed_nmf_objective_function
  // we use that for the first upper box in global optimization
  fx_module *relaxed_nmf_module=fx_submodule(module_, "relaxed_nmf");
  // module for the classic nmf objective
  fx_module *classic_nmf_module=fx_submodule(module_, "classic_nmf");
  

  new_dimension_=fx_param_int(module_, "new_dimension", 2);
  fx_set_param_int(gop_nmf_engine_module, "new_dimension", new_dimension_);
  fx_set_param_int(relaxed_nmf_module, "new_dimension", new_dimension_);
  fx_set_param_int(relaxed_nmf_box_tightener_module, "new_dimension", new_dimension_);
  fx_set_param_int(classic_nmf_module, "new_dimension", new_dimension_);

  
  // Transfer the matrix into row column pairs
  values_.Init();
	rows_.Init();
	columns_.Init();
	for(size_t i=0; i<data_matrix.n_rows(); i++) {
  	for(size_t j=0; j<data_matrix.n_cols(); j++) {
		  values_.PushBackCopy(data_matrix.get(i, j));
			rows_.PushBackCopy(i);
			columns_.PushBackCopy(j);
		}
	}
  num_of_rows_=*std::max_element(rows_.begin(), rows_.end())+1;
  num_of_columns_=*std::max_element(columns_.begin(), columns_.end())+1;
  fx_set_param_int(l_bfgs_module, "num_of_points", num_of_rows_+num_of_columns_);
  // initialize the lower and upper boxes
  lower_box_.Init(new_dimension_);
  lower_box_.SetAll(fx_param_double(module_, "lower_box", log(1e-7))); 
  upper_box_.Init(new_dimension_);
  upper_box_.SetAll(fx_param_double(module_, "upper_box", log(1.0)));

  
  Matrix lo, hi;
  lo.Init(new_dimension_, num_of_rows_+num_of_columns_);
  hi.Init(new_dimension_, num_of_rows_+num_of_columns_);
  lo.SetAll(lower_box_[0]);
  hi.SetAll(upper_box_[0]);
  relaxed_nmf_.Init(relaxed_nmf_module, rows_, columns_, 
                    values_, lo, hi);
  relaxed_nmf_optimizer_.Init(&relaxed_nmf_, l_bfgs_module);
  Matrix init_data;
  relaxed_nmf_.GiveInitMatrix(&init_data);
  relaxed_nmf_optimizer_.set_coordinates(init_data);
  relaxed_nmf_optimizer_.ComputeLocalOptimumBFGS();
  current_solution_= new Matrix();
  current_solution_->Copy(*relaxed_nmf_optimizer_.coordinates());
  relaxed_nmf_.ComputeObjective(
      *current_solution_,
      &objective_minimum_upper_bound_);
  objective_minimum_upper_bound_+=10;
  
/*
  classic_nmf_objective_.Init(classic_nmf_module, 
                             rows_,
                             columns_,
                             values_);
  classic_nmf_optimizer_.Init(&classic_nmf_objective_, l_bfgs_module);
  classic_nmf_objective_.GiveInitMatrix(&init_data);
  classic_nmf_optimizer_.set_coordinates(init_data);
  classic_nmf_optimizer_.ComputeLocalOptimumBFGS();
  current_solution_= new Matrix();
  current_solution_->Copy(*classic_nmf_optimizer_.coordinates());

  classic_nmf_objective_.ComputeObjective(*current_solution_, &objective_minimum_upper_box_);
  
  // we need to convert it to logs
  for(size_t i=0; i<current_solution_->n_cols(); i++) {
    for(size_t j=0; j<current_solution_->n_rows(); j++) {
      current_solution_->set(j, i, log(std::max(current_solution_->get(j, i), 1e-6)));
    }
  }
*/  
  relaxed_nmf_box_tightener_.Init(relaxed_nmf_box_tightener_module,
                                  rows_, 
                                  columns_, 
                                  values_,
                                  lower_box_, 
                                  upper_box_,
                                  1,
                                  objective_minimum_upper_bound_);
  //relaxed_nmf_box_tightener_.GiveInitMatrix(current_solution_);  
  box_tightener_optimizer_.Init(&relaxed_nmf_box_tightener_, l_bfgs_module);
  gop_nmf_engine_.Init(gop_nmf_engine_module, data_matrix);
}

void GopTightNmfEngine::TightenBounds() {
/*  for(size_t i=0; i< lower_box_.n_rows(); i++) {
    for(size_t j=0; j<lower_box_.n_cols(); j++) {
      // find the lower box for the i, j variable
      relaxed_nmf_box_tightener_.SetOptVarRowColumn(i, j);
      relaxed_nmf_box_tightener_.SetOptVarSign(1.0);
      box_tightener_optimizer_.set_coordinates(*current_solution_);
      box_tightener_optimizer_.Reset();
      box_tightener_optimizer_.ComputeLocalOptimumBFGS();
       current_solution_->CopyValues(*box_tightener_optimizer_.coordinates());
      // lower box update
      lower_box_.set(i, j, current_solution_->get(i, j));
      // find the upper box for the i, j variable
      relaxed_nmf_box_tightener_.SetOptVarSign(-1.0);
      box_tightener_optimizer_.set_coordinates(*current_solution_);
      box_tightener_optimizer_.Reset();
      box_tightener_optimizer_.ComputeLocalOptimumBFGS();
      current_solution_->CopyValues(*box_tightener_optimizer_.coordinates());
      // upper box update
      upper_box_.set(i, j, current_solution_->get(i, j));
    }
  }
*/
  relaxed_nmf_box_tightener_.SetOptVarSign(1.0);
  box_tightener_optimizer_.set_coordinates(*current_solution_);
  box_tightener_optimizer_.Reset();
  box_tightener_optimizer_.ComputeLocalOptimumBFGS();
  current_solution_->CopyValues(*box_tightener_optimizer_.coordinates());
  for(size_t j=0; j<current_solution_->n_rows(); j++) {
    double min_value=DBL_MAX;
    for(size_t i=0; i<current_solution_->n_cols(); i++) {
       if (current_solution_->get(j, i)<min_value) {
         min_value = current_solution_->get(j, i);
       }
    }
    NOTIFY("Minimum value for %i:%lg", j, min_value);
  }
  // lower box update
  // lower_box_;
  // find the upper box for the i, j variable
  relaxed_nmf_box_tightener_.SetOptVarSign(-1.0);
  box_tightener_optimizer_.set_coordinates(*current_solution_);
  box_tightener_optimizer_.Reset();
  box_tightener_optimizer_.ComputeLocalOptimumBFGS();
  current_solution_->CopyValues(*box_tightener_optimizer_.coordinates());
  // upper box update
  // upper_box_;
   for(size_t j=0; j<current_solution_->n_rows(); j++) {
    double max_value=-DBL_MAX;
    for(size_t i=0; i<current_solution_->n_cols(); i++) {
       if (current_solution_->get(j, i)>max_value) {
         max_value = current_solution_->get(j, i);
       }
    }
    NOTIFY("Maximum value for %i:%lg", j, max_value);
  }
  
  NOTIFY("Bounds tightened!!");
}

void GopTightNmfEngine::ComputeGlobalOptimum() {
  gop_nmf_engine_.ComputeGlobalOptimum();
}

#endif
#endif
