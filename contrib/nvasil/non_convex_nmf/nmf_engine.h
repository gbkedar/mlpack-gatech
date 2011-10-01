#ifndef NMF_ENGINE_H_
#define NMF_ENGINE_H_
#include "fastlib/fastlib.h"
#include "../l_bfgs/l_bfgs.h"
#include "nmf_objectives.h"

template<typename NmfObjective>
class NmfEngine {
 public:
	 void  Init(fx_module *module) {
 		 module_=module;
	   std::string data_file=fx_param_str_req(module_, "data_file");
     sdp_rank_=fx_param_int(module_,  "sdp_rank", 5);
		 new_dim_=fx_param_int(module_, "new_dimension", 3);
		 Matrix data_mat;
		 if (data::Load(data_file.c_str(), &data_mat)==false) {
       FATAL("Terminating...");
     }
     NOTIFY("Factoring a %i x %i matrix in %i x %i and %i x %i\n",
         data_mat.n_rows(), data_mat.n_cols(), data_mat.n_rows(), new_dim_,
         new_dim_, data_mat.n_cols());
		 PreprocessData(data_mat);
		 fx_module *opt_function_module=fx_submodule(module_, "optfun");
     fx_set_param_int(opt_function_module, "rank", sdp_rank_);
		 fx_set_param_int(opt_function_module, "new_dimension", new_dim_);
		 opt_function_.Init(opt_function_module, rows_, columns_, values_);
		 fx_module *l_bfgs_module=fx_submodule(module_, "l_bfgs");
     fx_set_param_bool(l_bfgs_module, "use_default_termination", false);
     Matrix init_data;
		 opt_function_.GiveInitMatrix(&init_data);
	   fx_set_param_int(l_bfgs_module, "num_of_points", init_data.n_cols());
		 fx_set_param_int(l_bfgs_module, "new_dimension", init_data.n_rows());
     engine_.Init(&opt_function_, l_bfgs_module);
	 	 engine_.set_coordinates(init_data);
     w_mat_.Init(new_dim_, num_of_columns_);
		 h_mat_.Init(new_dim_, num_of_rows_);
	 }
	 void Destruct() {
	   
	 };
	 void ComputeNmf() {
     Matrix init_data;
		 opt_function_.GiveInitMatrix(&init_data);
		 engine_.set_coordinates(init_data);
	   engine_.ComputeLocalOptimumBFGS();
     Matrix result;
     engine_.CopyCoordinates(&result);
     //OptUtils::NonNegativeProjection(&result);
     w_mat_.CopyColumnFromMat(0, num_of_rows_, num_of_columns_ , result);
     h_mat_.CopyColumnFromMat(0, 0, num_of_rows_, result);

     // now compute reconstruction error
     Matrix v_rec;
     la::MulTransAInit(h_mat_, w_mat_, &v_rec);
     double error=0;
     double v_sum=0;
     for(size_t i=0; i<values_.size(); i++) {
       size_t r=rows_[i];
       size_t c=columns_[i];
       error+=fabs(v_rec.get(r, c)-values_[i]);
       v_sum+=values_[i];
     }
     reconstruction_error_=error*100/v_sum;
     NOTIFY("Reconstruction error: %lg%%\n", reconstruction_error_);
	 }
   
	 void GetW(Matrix *w_mat) {
	   w_mat->Copy(w_mat_);
	 }
	 
   void GetH(Matrix *h_mat) {
	   h_mat->Copy(h_mat_);
	 }
	 
   double reconstruction_error() {
    return reconstruction_error_;
   }
   
 private:
	fx_module *module_;
  LBfgs<NmfObjective> engine_;
	NmfObjective opt_function_;
	ArrayList<size_t> rows_;
	ArrayList<size_t> columns_;
	ArrayList<double> values_;
	size_t new_dim_;
	size_t sdp_rank_;
	Matrix w_mat_;
	Matrix h_mat_;
	size_t num_of_rows_; // number of unique rows, otherwise the size of W
	size_t num_of_columns_; // number of unique columns, otherwise the size of H
  double reconstruction_error_;
 
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
