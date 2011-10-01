/*
 * =====================================================================================
 * 
 *       Filename:  geometric_nmf_engine.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  07/04/2008 10:52:01 AM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

#ifndef GEOMETRIC_NMF_H_
#define GEOMTETRIC_NMF_H_
#include "geometric_nmf.h"

template<typename GeometricNmfObjective>
class GeometricNmfEngine {
 public:
   GeometricNmfEngine() {
     engine_=NULL;
   }
   ~GeometricNmfEngine() {
     if (engine_!=NULL) {
       delete engine_;
     }
   }
	 void  Init(fx_module *module) {
 		 module_=module;
	   std::string data_file=fx_param_str_req(module_, "data_file");
		 new_dim_=fx_param_int(module_, "new_dim", 3);
		 Matrix data_mat;
		 if (data::Load(data_file.c_str(), &data_mat)==false) {
       FATAL("Terminating...");
     }
     NOTIFY("Factoring a %i x %i matrix in %i x %i and %i x %i\n",
         data_mat.n_rows(), data_mat.n_cols(), data_mat.n_rows(), new_dim_,
         new_dim_, data_mat.n_cols());
		 PreprocessData(data_mat);
		 fx_module *opt_function_module=fx_submodule(module_, "optfun");
		 fx_set_param_int(opt_function_module, "new_dim", new_dim_);
		 opt_function_.Init(opt_function_module, rows_, columns_, values_);
		 l_bfgs_module_=fx_submodule(module_, "l_bfgs");
	 }
	 void Destruct() {
     if (engine_!=NULL) {
	     delete engine_;
       engine_ = NULL;
     }
     GeometricNmfObjective  opt_function_;
	   rows_.Renew();
	   columns_.Renew();
	   values_.Renew();
	   w_mat_.Destruct();
	   h_mat_.Destruct();

	 };
	 void ComputeNmf() {
     Matrix result;
		 opt_function_.GiveInitMatrix(&result);
	   fx_set_param_int(l_bfgs_module_, "num_of_points", result.n_cols());
		 fx_set_param_int(l_bfgs_module_, "new_dimension", result.n_rows());
     engine_=new LBfgs<GeometricNmfObjective>();
     engine_->Init(&opt_function_, l_bfgs_module_);
     engine_->set_coordinates(result);
     result.Destruct();
     engine_->ComputeLocalOptimumBFGS();
     engine_->GetResults(&result);
     delete engine_;
        
		 for(size_t i=0; i<num_of_rows_+num_of_columns_; i++) {
       for(size_t j=0; j<result.n_rows(); j++) {
         result.set(j, i, exp(result.get(j, i)));
       }
     }
     w_mat_.Init(new_dim_, num_of_rows_);
		 h_mat_.Init(new_dim_, num_of_columns_);
     w_mat_.CopyColumnFromMat(0, 0, num_of_rows_, result); 
     h_mat_.CopyColumnFromMat(0, 
         num_of_rows_, num_of_columns_, result);
 
     data::Save("result.csv", result);
     // now compute reconstruction error
     Matrix v_rec;
     la::MulTransAInit(w_mat_, h_mat_, &v_rec);
   
     double error=0;
     double v_sum=0;
     for(size_t i=0; i<values_.size(); i++) {
       size_t r=rows_[i];
       size_t c=columns_[i];
       error+=fabs(v_rec.get(r, c)-values_[i]);
       v_sum+=values_[i];
     }
     NOTIFY("Reconstruction error: %lg%%\n", error*100/v_sum);
	 }
   
	 void GetW(Matrix *w_mat) {
	   w_mat->Copy(w_mat_);
	 }
	 void GetH(Matrix *h_mat) {
	   h_mat->Copy(h_mat_);
	 }
	 
 private:
	fx_module *module_;
  fx_module *l_bfgs_module_;
  LBfgs<GeometricNmfObjective> *engine_;
	GeometricNmfObjective  opt_function_;
	ArrayList<size_t> rows_;
	ArrayList<size_t> columns_;
	ArrayList<double> values_;
	size_t new_dim_;
	Matrix w_mat_;
	Matrix h_mat_;
	size_t num_of_rows_; // number of unique rows, otherwise the size of W
	size_t num_of_columns_; // number of unique columns, otherwise the size of H
 
	void PreprocessData(Matrix &data_mat) {
	  values_.Init();
		rows_.Init();
		columns_.Init();
		for(size_t i=0; i<data_mat.n_rows(); i++) {
		  for(size_t j=0; j< data_mat.n_cols(); j++) {
			  values_.PushBackCopy(data_mat.get(i, j));
				rows_.PushBackCopy(i);
				columns_.PushBackCopy(j);
			}
		}
    num_of_rows_=*std::max_element(rows_.begin(), rows_.end())+1;
    num_of_columns_=*std::max_element(columns_.begin(), columns_.end())+1;
	}
};

#endif
