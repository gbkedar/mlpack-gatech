/**
 * @author Hua Ouyang
 *
 * @file regmin.h
 *
 * This head file contains functions for training and prediction of 
 * regulazied risk minimization problems.
 * Supported learner type:SVM_C, SVM_R, SVM_Q
 *
 * @see opt_smo.h 
 * @see opt_sgd.h 
 * @see opt_md.h 
 * @see opt_tgd.h 
 */

#ifndef U_REGMIN_H
#define U_REGMIN_H

#include "fastlib/fastlib.h"
#include "regmin_data.h"
#include "opt_smo.h"
#include "opt_sgd.h"
#include "opt_md.h"
#include "opt_tgd.h"

#include <typeinfo>


/**
* Class for SVM
*/
template<typename TKernel>
class SVM {

 private:
  /** 
   * Type id of the SVM learner: 
   *  0:SVM Classification (svm_c);
   *  1:SVM Regression (svm_r);
   *  2:SVM quantile estimation (svm_q);
   * Developers may add more learner types if necessary
   */
  int learner_typeid_;
  // Optimization method: smo, lasvm, sgd, tgd, cd, pegasos, rivanov, hcy, fw, mfw, sfw, smd, mfw, par, sparsereg
  String opt_method_;
  /* array of models for storage of the 2-class(binary) classifiers 
     Need to train num_classes_*(num_classes_-1)/2 binary models */
  struct SVM_MODELS {
    /* bias term in each binary model */
    double bias_;
    /* all coefficients (alpha*y) of the binary dataset, not necessarily thoes of SVs */
    ArrayList<double> coef_;
    /* the slope w */
    //Vector w_;
    ArrayList<NZ_entry> w_;
    /* scale for w*/
    double scale_w_; // Use it if w's scaling is not done in training session
  };
  ArrayList<SVM_MODELS> models_;

  /* list of labels, double type, but may be converted to integers.
     e.g. [0.0,1.0,2.0] for a 3-class dataset */
  ArrayList<double> train_labels_list_;
  /* array of label indices, after grouping. e.g. [c1[0,5,6,7,10,13,17],c2[1,2,4,8,9],c3[...]]*/
  ArrayList<size_t> train_labels_index_;
  /* counted number of label for each class. e.g. [7,5,8]*/
  ArrayList<size_t> train_labels_ct_;
  /* start positions of each classes in the training label list. e.g. [0,7,12] */
  ArrayList<size_t> train_labels_startpos_;
  
  /* total set of support vectors and their coefficients */
  struct NZ_entry **sv_entries_; // store the SVs
  struct NZ_entry *sv_nz_pool_; // store the nonzero entries of SVs for testing
  Matrix sv_coef_;
  ArrayList<bool> trainset_sv_indicator_;

  /* total number of support vectors */
  size_t total_num_sv_;
  /* support vector list to store the indices (in the training set) of support vectors */
  ArrayList<size_t> sv_index_;
  /* start positions of each class of support vectors, in the support vector list */
  ArrayList<size_t> sv_list_startpos_;
  /* counted number of support vectors for each class */
  ArrayList<size_t> sv_list_ct_;

  /* SVM parameters */
  struct PARAMETERS {
    TKernel kernel_;
    String kernelname_;
    int kerneltypeid_;
    int b_;
    double C_;
    // for SVM_C of unbalanced data
    double Cp_; // C for y==1
    double Cn_; // C for y==-1
    // for nu-SVM
    double nu_;
    // for SVM_R
    double epsilon_;
    // working set selection scheme of SMO, 1 for 1st order expansion; 2 for 2nd order expansion
    double wss_;
    // whether do L1-SVM (1) or L2-SVM (2)
    int hinge_;
    // accuracy for the optimization stopping creterion
    double accuracy_;
    // number of iterations
    size_t n_iter_;
    // number of epochs for stochastic algorithms
    size_t n_epochs_;
  };
  PARAMETERS param_;
  
  /* number of data samples */
  size_t n_data_; 
  /* number of classes in the training set */
  int num_classes_;
  /* number of binary models to be trained, i.e. num_classes_*(num_classes_-1)/2 */
  int num_models_;
  int num_features_;

  size_t max_line_length_;;
  char *line_;

 public:
  typedef TKernel Kernel;
  class SMO<Kernel>;
  class SGD<Kernel>;
  class MD<Kernel>;
  class TGD<Kernel>;

  void Init(int learner_typeid, Dataset_sl& dataset, datanode *module);
  void InitTrain(int learner_typeid, Dataset_sl& dataset, datanode *module);
  void GetLabels(Dataset_sl& dataset,
		 ArrayList<double> &labels_list,
		 ArrayList<size_t> &labels_index,
		 ArrayList<size_t> &labels_ct,
		 ArrayList<size_t> &labels_startpos);

  double Predict(int learner_typeid, NZ_entry *test_vec);
  void BatchPredict(int learner_typeid, Dataset_sl& testset, String predictedvalue_filename);
  void LoadModelBatchPredict(int learner_typeid, Dataset_sl& testset, String model_filename, String predictedvalue_filename);

 private:
  void SVM_C_Train_(int learner_typeid, Dataset_sl& dataset, datanode *module);
  void SVM_R_Train_(int learner_typeid, Dataset_sl& dataset, datanode *module);
  void SVM_Q_Train_(int learner_typeid, Dataset_sl& dataset, datanode *module);
  double SVM_C_Predict_(NZ_entry *test_vec);
  double SVM_R_Predict_(NZ_entry *test_vec);
  double SVM_Q_Predict_(NZ_entry *test_vec);

  void SaveModel_(int learner_typeid, String model_filename);
  void LoadModel_(int learner_typeid, String model_filename);
  char *ReadLine(FILE *fp);
};

template<typename TKernel>
char * SVM<TKernel>::ReadLine(FILE *fp_in) {
  size_t length;
  if ( fgets(line_, max_line_length_, fp_in)==NULL ) {
    return NULL;
  }
  while ( strrchr(line_,'\n')==NULL ) {
    max_line_length_ *= 2;
    line_ = (char *) realloc(line_, max_line_length_);
    length = (size_t) strlen(line_);
    if ( fgets(line_+length, max_line_length_-length, fp_in)==NULL ) {
      break;
    }
  }
  return line_;
}

template<typename TKernel>
void SVM<TKernel>::GetLabels(Dataset_sl &dataset,
			     ArrayList<double> &labels_list,
			     ArrayList<size_t> &labels_index,
			     ArrayList<size_t> &labels_ct,
			     ArrayList<size_t> &labels_startpos) {
  size_t i = 0;
  //size_t label_row_idx = matrix_.n_rows() - 1; // the last row is for labels
  size_t n_points = dataset.n_points;
  size_t n_labels = 0;

  double current_label;

  // these Arraylists need initialization before-hand
  labels_list.Renew();
  labels_index.Renew();
  labels_ct.Renew();
  labels_startpos.Renew();

  labels_index.Init(n_points);
  labels_list.Init();
  labels_ct.Init();
  labels_startpos.Init();

  ArrayList<size_t> labels_temp;
  labels_temp.Init(n_points);
  labels_temp[0] = 0;

  labels_list.PushBack() = dataset.y[0];
  labels_ct.PushBack() = 1;
  n_labels++;

  for (i = 1; i < n_points; i++) {
    current_label = dataset.y[i];
    size_t j = 0;
    for (j = 0; j < n_labels; j++) {
      if (current_label == labels_list[j]) {
        labels_ct[j]++;
	break;
      }
    }
    labels_temp[i] = j;
    if (j == n_labels) { // new label
      labels_list.PushBack() = current_label; // add new label to list
      labels_ct.PushBack() = 1;
      n_labels++;
    }
  }
  
  labels_startpos.PushBack() = 0;
  for(i = 1; i < n_labels; i++){
    labels_startpos.PushBack() = labels_startpos[i-1] + labels_ct[i-1];
  }

  for(i = 0; i < n_points; i++) {
    labels_index[labels_startpos[labels_temp[i]]] = i;
    labels_startpos[labels_temp[i]]++;
  }

  labels_startpos[0] = 0;
  for(i = 1; i < n_labels; i++) {
    labels_startpos[i] = labels_startpos[i-1] + labels_ct[i-1];
  }

  dataset.n_classes = n_labels;

  labels_temp.Clear();
}


/**
* SVM initialization
*
* @param: labeled training set or testing set
* @param: number of classes (different labels) in the data set
* @param: module name
*/
template<typename TKernel>
void SVM<TKernel>::Init(int learner_typeid, Dataset_sl& dataset, datanode *module){
  learner_typeid_ = learner_typeid;

  opt_method_ = fx_param_str(NULL, "opt", "smo"); // optimization method: default using SMO

  n_data_ = dataset.n_points;
  
  train_labels_list_.Init();
  train_labels_index_.Init();
  train_labels_ct_.Init();
  train_labels_startpos_.Init();

  /* 1.Find the # of classes of the training set;
     2.Group labels, split the training dataset for training bi-class SVM classifiers */
  GetLabels(dataset, train_labels_list_, train_labels_index_, train_labels_ct_, train_labels_startpos_);
  num_classes_ = dataset.n_classes;
  
  if (learner_typeid == 0) { /* for multiclass SVM classificatioin*/
    num_models_ = num_classes_ * (num_classes_-1) / 2;
    sv_list_startpos_.Init(num_classes_);
    sv_list_ct_.Init(num_classes_);
  }
  else { /* for other SVM learners */
    num_classes_ = 2; // dummy #, only meaningful in SaveModel and LoadModel

    num_models_ = 1;
    sv_list_startpos_.Init();
    sv_list_ct_.Init();
  }

  models_.Init();
  sv_index_.Init();
  total_num_sv_ = 0;

  /* bool indicators FOR THE TRAINING SET: is/isn't a support vector */
  /* Note: it has the same index as the training !!! */
  trainset_sv_indicator_.Init(n_data_);
  for (size_t i=0; i<n_data_; i++) {
    trainset_sv_indicator_[i] = false;
  }

  param_.kernel_.Init(fx_submodule(module, "kernel"));
  param_.kernel_.GetName(&param_.kernelname_);
  param_.kerneltypeid_ = param_.kernel_.GetTypeId();
  // budget parameter, contorls # of support vectors; default: # of data samples (use all)
  param_.b_ = fx_param_int(NULL, "b", dataset.n_points);
  // working set selection scheme. default: 1st order expansion
  param_.wss_ = fx_param_int(NULL, "wss", 1);
  // whether do L1-SVM(1) or L2-SVM (2)
  param_.hinge_ = fx_param_int(NULL, "hinge", 1); // default do L1-SVM
  // accuracy for optimization
  param_.accuracy_ = fx_param_double(NULL, "accuracy", 1e-4);
  // number of iterations
  param_.n_iter_ = fx_param_int(NULL, "n_iter", n_data_);
  // number of iterations
  param_.n_epochs_ = fx_param_int(NULL, "n_epochs", 0);

  // tradeoff parameter for C-SV
  param_.C_ = fx_param_double(NULL, "c", 10.0);
  param_.Cp_ = fx_param_double(NULL, "c_p", param_.C_);
  param_.Cn_ = fx_param_double(NULL, "c_n", param_.C_);

  // portion of SVs for nu-SVM, need 0< nu <=1
  param_.nu_ = fx_param_double(NULL, "nu", 0.1);

  if (learner_typeid == 1) { // for SVM_R only
    // the "epsilon", default: 0.1
    param_.epsilon_ = fx_param_double(NULL, "epsilon", 0.1);
  }
  else if (learner_typeid == 2) { // SVM_Q
    // TODO
  }

  line_ = NULL;
  max_line_length_ = 1024;
}

/**
* Initialization(data dependent) and training for SVM learners
*
* @param: typeid of the learner
* @param: number of classes (different labels) in the training set
* @param: module name
*/
template<typename TKernel>
void SVM<TKernel>::InitTrain(int learner_typeid, Dataset_sl& dataset, datanode *module) {
  Init(learner_typeid, dataset, module);
  if (learner_typeid == 0) { // Multiclass SVM Clssification
    SVM_C_Train_(learner_typeid, dataset, module);
  }
  else if (learner_typeid == 1) { // SVM Regression
    SVM_R_Train_(learner_typeid, dataset, module);
  }
  else if (learner_typeid == 2) { // SVM Quantile Estimation
    SVM_Q_Train_(learner_typeid, dataset, module);
  }
  
  /* Save models to file "svm_model" */
  SaveModel_(learner_typeid, "svm_model"); // TODO: param_req, and for CV mode
  // TODO: calculate training error
}


/**
* Training for Multiclass SVM Clssification, using One-vs-One method
*
* @param: type id of the learner
* @param: training set
* @param: number of classes of the training set
* @param: module name
*/
template<typename TKernel>
void SVM<TKernel>::SVM_C_Train_(int learner_typeid, Dataset_sl &dataset, datanode *module) {
  /* Train num_classes*(num_classes-1)/2 binary class(labels:-1, 1) models */
  size_t ct = 0;
  size_t i, j;
  for (i = 0; i < num_classes_; i++) {
    for (j = i+1; j < num_classes_; j++) {
      models_.PushBack();
      /* Construct dataset consists of two classes i and j (reassign labels 1 and -1) */
      Dataset_sl dataset_bi;
      dataset_bi.n_points = train_labels_ct_[i]+train_labels_ct_[j];
      dataset_bi.n_features = dataset.n_features;
      dataset_bi.x = Malloc(struct NZ_entry *, dataset_bi.n_points);
      dataset_bi.y = Malloc(double, dataset_bi.n_points);
      ArrayList<size_t> dataset_bi_index;
      dataset_bi_index.Init(dataset_bi.n_points);
      // for class 1
      for (size_t m = 0; m < train_labels_ct_[i]; m++) {
	dataset_bi.x[m] = dataset.x[train_labels_index_[train_labels_startpos_[i]+m]];
	dataset_bi.y[m] = 1;
	dataset_bi_index[m] = train_labels_index_[train_labels_startpos_[i]+m];
      }
      // flor class -1
      for (size_t n = 0; n < train_labels_ct_[j]; n++) {
	dataset_bi.x[n+train_labels_ct_[i]] = dataset.x[train_labels_index_[train_labels_startpos_[j]+n]];
	dataset_bi.y[n+train_labels_ct_[i]] = -1;
	dataset_bi_index[n+train_labels_ct_[i]] = train_labels_index_[train_labels_startpos_[j]+n];
      }

      if (opt_method_== "smo") {
	// Initialize SMO parameters
	ArrayList<double> param_feed_db;
	param_feed_db.Init();
	param_feed_db.PushBack() = param_.b_;
	param_feed_db.PushBack() = param_.Cp_;
	param_feed_db.PushBack() = param_.Cn_;
	param_feed_db.PushBack() = param_.hinge_;
	param_feed_db.PushBack() = param_.wss_;
	param_feed_db.PushBack() = param_.n_iter_;
	param_feed_db.PushBack() = param_.accuracy_;
	SMO<Kernel> smo;
	smo.InitPara(learner_typeid, param_feed_db);

	// Initialize kernel
	smo.kernel().Init(fx_submodule(module, "kernel"));

	// 2-classes SVM training using SMO
	fx_timer_start(NULL, "train_smo");
	smo.Train(learner_typeid, dataset_bi);
	fx_timer_stop(NULL, "train_smo");

	// Get the trained bi-class model
	models_[ct].coef_.Init(); // alpha*y
	models_[ct].bias_ = smo.Bias(); // bias
	//models_[ct].w_.Init(0); // for linear classifiers only. not used here
	smo.GetSV(dataset_bi_index, models_[ct].coef_, trainset_sv_indicator_); // get support vectors
      }
      else if (opt_method_== "sgd") {
	// Initialize SGD parameters
	ArrayList<double> param_feed_db;
	param_feed_db.Init();
	param_feed_db.PushBack() = param_.Cp_;
	param_feed_db.PushBack() = param_.Cn_;
	param_feed_db.PushBack() = param_.kerneltypeid_== 0 ? 0.0: 1.0;
	param_feed_db.PushBack() = param_.n_epochs_;
	param_feed_db.PushBack() = param_.n_iter_;
	param_feed_db.PushBack() = param_.accuracy_;
	SGD<Kernel> sgd;
	sgd.InitPara(learner_typeid, param_feed_db);

	// Initialize kernel
	sgd.kernel().Init(fx_submodule(module, "kernel"));

	// 2-classes SVM training using SGD
	fx_timer_start(NULL, "train_sgd");
	sgd.Train(learner_typeid, dataset_bi);
	fx_timer_stop(NULL, "train_sgd");
	// Get the trained bi-class model
	models_[ct].coef_.Init(); // alpha*y, used for nonlinear SVM only
	if (param_.kerneltypeid_== 0) { // linear SVM
	  sgd.GetW(models_[ct].w_);
	  models_[ct].scale_w_ = sgd.ScaleW(); // scale of w for linear SVM. Use it if w's scaling is not done in training session
	}
	else { // nonlinear SVM
	  sgd.GetSV(dataset_bi_index, models_[ct].coef_, trainset_sv_indicator_); // get support vectors
	  //models_[ct].w_.Init(0); // for linear SVM only. not used here
	}
	models_[ct].bias_ = sgd.Bias(); // bias
      }
      else if (opt_method_== "md") {
	// Initialize MD parameters
	ArrayList<double> param_feed_db;
	param_feed_db.Init();
	param_feed_db.PushBack() = param_.Cp_;
	param_feed_db.PushBack() = param_.Cn_;
	param_feed_db.PushBack() = param_.n_epochs_;
	param_feed_db.PushBack() = param_.n_iter_;
	param_feed_db.PushBack() = param_.accuracy_;
	MD<Kernel> md;
	md.InitPara(learner_typeid, param_feed_db);

	// Initialize kernel
	md.kernel().Init(fx_submodule(module, "kernel"));

	// 2-classes SVM training using MD
	fx_timer_start(NULL, "train_md");
	md.Train(learner_typeid, dataset_bi);
	fx_timer_stop(NULL, "train_md");
	// Get the trained bi-class model
	models_[ct].coef_.Init(); // alpha*y, not used here
	md.GetW(models_[ct].w_);
	models_[ct].scale_w_ = md.ScaleW(); // scale of w for linear SVM. Use it if w's scaling is not done in training session
      }
      else if (opt_method_== "tgd") {
	// Initialize TGD parameters
	ArrayList<double> param_feed_db;
	param_feed_db.Init();
	param_feed_db.PushBack() = param_.Cp_;
	param_feed_db.PushBack() = param_.Cn_;
	param_feed_db.PushBack() = param_.n_epochs_;
	param_feed_db.PushBack() = param_.n_iter_;
	param_feed_db.PushBack() = param_.accuracy_;
	TGD<Kernel> tgd;
	tgd.InitPara(learner_typeid, param_feed_db);

	// Initialize kernel
	tgd.kernel().Init(fx_submodule(module, "kernel"));

	// 2-classes SVM training using TGD
	fx_timer_start(NULL, "train_tgd");
	tgd.Train(learner_typeid, dataset_bi);
	fx_timer_stop(NULL, "train_tgd");
	// Get the trained bi-class model
	models_[ct].coef_.Init(); // alpha*y, not used here
	tgd.GetW(models_[ct].w_);
	models_[ct].scale_w_ = tgd.ScaleW(); // scale of w for linear SVM. Use it if w's scaling is not done in training session
      }
      else {
	fprintf(stderr, "ERROR!!! Unknown optimization method!\n");
      }

      ct++;
    }
  }
  
  /* Get total set of SVs from all the binary models */
  size_t k;
  sv_list_startpos_[0] = 0;

  for (i = 0; i < num_classes_; i++) {
    ct = 0;
    for (j = 0; j < train_labels_ct_[i]; j++) {
      if (trainset_sv_indicator_[ train_labels_index_[train_labels_startpos_[i]+j] ]) {
	sv_index_.PushBack() = train_labels_index_[train_labels_startpos_[i]+j];
	total_num_sv_++;
	ct++;
      }
    }
    sv_list_ct_[i] = ct;
    if (i >= 1)
      sv_list_startpos_[i] = sv_list_startpos_[i-1] + sv_list_ct_[i-1];    
  }
  //sv_.Init(num_features_, total_num_sv_);
  sv_entries_ = Malloc(struct NZ_entry *, total_num_sv_);
  for (i = 0; i < total_num_sv_; i++) {
    sv_entries_[i] = (dataset.x)[sv_index_[i]];
  }

  /* Get the matrix sv_coef_ which stores the coefficients of all sets of SVs */
  /* i.e. models_[x].coef_ -> sv_coef_ */
  size_t ct_model = 0;
  size_t p;
  sv_coef_.Init(num_classes_-1, total_num_sv_);
  sv_coef_.SetZero();
  for (i = 0; i < num_classes_; i++) {
    for (j = i+1; j < num_classes_; j++) {
      p = sv_list_startpos_[i];
      for (k = 0; k < train_labels_ct_[i]; k++) {
	if (trainset_sv_indicator_[ train_labels_index_[train_labels_startpos_[i]+k] ]) {
	  sv_coef_.set(j-1, p++, models_[ct_model].coef_[k]);
	}
      }
      p = sv_list_startpos_[j];
      for (k = 0; k < train_labels_ct_[j]; k++) {
	if (trainset_sv_indicator_[ train_labels_index_[train_labels_startpos_[j]+k] ]) {
	  sv_coef_.set(i, p++, models_[ct_model].coef_[train_labels_ct_[i] + k]);
	}
      }
      ct_model++;
    }
  }
}

/**
* Training for SVM Regression
*
* @param: type id of the learner
* @param: training set
* @param: module name
*/
template<typename TKernel>
void SVM<TKernel>::SVM_R_Train_(int learner_typeid, Dataset_sl &dataset, datanode *module) {
  size_t i;
  ArrayList<size_t> dataset_index;
  dataset_index.Init(n_data_);
  for (i=0; i<n_data_; i++)
    dataset_index[i] = i;

  models_.PushBack();

  if (opt_method_== "smo") {
    // Initialize SMO parameters
    ArrayList<double> param_feed_db;
    param_feed_db.Init();
    param_feed_db.PushBack() = param_.b_;
    param_feed_db.PushBack() = param_.C_;
    param_feed_db.PushBack() = param_.epsilon_;
    param_feed_db.PushBack() = param_.wss_;
    param_feed_db.PushBack() = param_.n_iter_;
    param_feed_db.PushBack() = param_.accuracy_;
    SMO<Kernel> smo;
    smo.InitPara(learner_typeid, param_feed_db);
    
    // Initialize kernel
    smo.kernel().Init(fx_submodule(module, "kernel"));

    // SVM_R Training using SMO
    smo.Train(learner_typeid, dataset);
    
    // Get the trained model
    models_[0].bias_ = smo.Bias(); // bias
    models_[0].coef_.Init(); // alpha*y
    //models_[0].w_.Init(0); // not using
    smo.GetSV(dataset_index, models_[0].coef_, trainset_sv_indicator_); // get support vectors
  }
  else if (opt_method_== "sgd") {
    // Initialize SGD parameters
    ArrayList<double> param_feed_db;
    param_feed_db.Init();
    param_feed_db.PushBack() = param_.Cp_;
    param_feed_db.PushBack() = param_.Cn_;
    param_feed_db.PushBack() = param_.kerneltypeid_== 0 ? 0.0: 1.0;
    SGD<Kernel> sgd;
    sgd.InitPara(learner_typeid, param_feed_db);
    
    // Initialize kernel
    sgd.kernel().Init(fx_submodule(module, "kernel"));

    // SVM_R Training using SGD
    sgd.Train(learner_typeid, dataset);
    
    // Get the trained model
    models_[0].bias_ = sgd.Bias(); // bias
    //models_[0].w_.Copy(*(sgd.GetW())); // w
    sgd.GetW(models_[0].w_);
    models_[0].scale_w_ = sgd.ScaleW(); // scale of w for linear SVM. Use it if w's scaling is not done in training session
    models_[0].coef_.Init(0); // not using
  }
  else {
    fprintf(stderr, "ERROR!!! Unknown optimization method!");
  }
  
  /* Get index list of support vectors */
  for (i = 0; i < n_data_; i++) {
    if (trainset_sv_indicator_[i]) {
      sv_index_.PushBack() = i;
      total_num_sv_++;
    }
  }

  /* Get support vecotors and coefficients */
  //sv_.Init(num_features_, total_num_sv_);
  sv_entries_ = Malloc(struct NZ_entry*, total_num_sv_);
  for (i = 0; i < total_num_sv_; i++) {
    sv_entries_[i] = dataset.x[sv_index_[i]];
  }

  /*
  for (i = 0; i < total_num_sv_; i++) {
    Vector source, dest;
    sv_.MakeColumnVector(i, &dest);
    // last row of dataset is for labels
    dataset.matrix().MakeColumnSubvector(sv_index_[i], 0, num_features_, &source); 
    dest.CopyValues(source);
  }
  */

  sv_coef_.Init(1, total_num_sv_);
  for (i = 0; i < total_num_sv_; i++) {
    sv_coef_.set(0, i, models_[0].coef_[i]);
  }
  
}

/**
* Training for SVM Quantile Estimation
*
* @param: type id of the learner
* @param: training set
* @param: module name
*/
template<typename TKernel>
void SVM<TKernel>::SVM_Q_Train_(int learner_typeid, Dataset_sl &dataset, datanode *module) {
  // TODO
}


/**
* SVM prediction for one testing vector
*
* @param: type id of the learner
* @param: testing vector
*
* @return: predited value
*/
template<typename TKernel>
double SVM<TKernel>::Predict(int learner_typeid, struct NZ_entry *test_vec) {
  double predicted_value = INFINITY;
  if (learner_typeid == 0) { // Multiclass SVM Clssification
    predicted_value = SVM_C_Predict_(test_vec);
  }
  else if (learner_typeid == 1) { // SVM Regression
    predicted_value = SVM_R_Predict_(test_vec);
  }
  else if (learner_typeid == 2) { // SVM Quantile Estimation
    predicted_value = SVM_Q_Predict_(test_vec);
  }
  return predicted_value;
}

/**
* Multiclass SVM classification for one testing vector
*
* @param: testing vector
*
* @return: a label (double-type-integer, e.g. 1.0, 2.0, 3.0)
*/
template<typename TKernel>
double SVM<TKernel>::SVM_C_Predict_(struct NZ_entry *test_vec) {
  size_t i, j, k;
  ArrayList<double> keval;
  keval.Init(total_num_sv_);
  if (opt_method_!="sgd" || param_.kerneltypeid_ != 0) {
    for (i = 0; i < total_num_sv_; i++) {
      keval[i] = param_.kernel_.Eval(test_vec, sv_entries_[i]);
    }
  }
  ArrayList<double> values;
  values.Init(num_models_);
  size_t ct = 0;
  double sum = 0.0;
  for (i = 0; i < num_classes_; i++) {
    for (j = i+1; j < num_classes_; j++) {
      if (opt_method_== "smo") {
	sum = 0.0;
	for (k = 0; k < sv_list_ct_[i]; k++) {
	  sum += sv_coef_.get(j-1, sv_list_startpos_[i]+k) * keval[sv_list_startpos_[i]+k];
	}
	for (k = 0; k < sv_list_ct_[j]; k++) {
	  sum += sv_coef_.get(i, sv_list_startpos_[j]+k) * keval[sv_list_startpos_[j]+k];
	}
	sum += models_[ct].bias_;
      }
      else if (opt_method_== "sgd") {
	if (param_.kerneltypeid_== 0) { // linear
	  sum = SparseDot(models_[ct].w_, test_vec);
	  sum *= models_[ct].scale_w_; // Use this if scaling of w is not done in the training session
	}
	else { //nonlinear
	  sum = 0.0;
	  for (k = 0; k < sv_list_ct_[i]; k++) {
	    sum += sv_coef_.get(j-1, sv_list_startpos_[i]+k) * keval[sv_list_startpos_[i]+k];
	  }
	  for (k = 0; k < sv_list_ct_[j]; k++) {
	    sum += sv_coef_.get(i, sv_list_startpos_[j]+k) * keval[sv_list_startpos_[j]+k];
	  }
	}
	sum += models_[ct].bias_;
      }
      else if (opt_method_== "md" || opt_method_== "tgd") {
	sum = SparseDot(models_[ct].w_, test_vec);
	//sum *= models_[ct].scale_w_; // Use this if scaling of w is not done in the training session
      }
      values[ct] = sum;
      ct++;
    }
  }

  ArrayList<size_t> vote;
  vote.Init(num_classes_);
  for (i = 0; i < num_classes_; i++) {
    vote[i] = 0;
  }
  ct = 0;
  for (i = 0; i < num_classes_; i++) {
    for (j = i+1; j < num_classes_; j++) {
      if(values[ct] > 0.0) { // label 1 in bi-classifiers (for i=...)
	vote[i] = vote[i] + 1;
      }
      else {  // label -1 in bi-classifiers (for j=...)
	vote[j] = vote[j] + 1;
      }
      ct++;
    }
  }
  size_t vote_max_idx = 0;
  for (i = 1; i < num_classes_; i++) {
    if (vote[i] >= vote[vote_max_idx]) {
      vote_max_idx = i;
    }
  }
  return train_labels_list_[vote_max_idx];
}

/**
* SVM Regression Prediction for one testing vector
*
* @param: testing vector
*
* @return: predicted regression value
*/
template<typename TKernel>
double SVM<TKernel>::SVM_R_Predict_(struct NZ_entry *test_vec) {
  size_t i;
  double sum = 0.0;
  if (opt_method_== "smo") {
    for (i = 0; i < total_num_sv_; i++) {
      //sum += sv_coef_.get(0, i) * param_.kernel_.Eval(datum.ptr(), sv_.GetColumnPtr(i), num_features_);
      sum += sv_coef_.get(0, i) * param_.kernel_.Eval(test_vec, sv_entries_[i]);
    }
  }
  else if (opt_method_== "sgd") {
    // TODO
  }
  sum += models_[0].bias_;
  return sum;
}

/**
* SVM Quantile Estimation Prediction for one testing vector
*
* @param: testing vector
*
* @return: estimated quantile value (the support)
*/
template<typename TKernel>
double SVM<TKernel>::SVM_Q_Predict_(struct NZ_entry *test_vec) {
  // TODO
  return 0.0;
}



/**
* Online batch classification for multiple testing vectors. No need to load model file, 
* since models are already in RAM.
*
* Note: for test set, if no true test labels provided, just put some dummy labels 
* (e.g. all -1) in the last row of testset
*
* @param: type id of the learner
* @param: testing set
* @param: file name of the testing data
*/
template<typename TKernel>
void SVM<TKernel>::BatchPredict(int learner_typeid, Dataset_sl& testset, String predictedvalue_filename) {
  FILE *fp = fopen(predictedvalue_filename, "w");
  double predictedvalue;
  if (fp == NULL) {
    fprintf(stderr, "Cannot save predicted values to file!\n");
    return;
  }
  size_t err_ct = 0;
  for (size_t i = 0; i < testset.n_points; i++) {
    predictedvalue = Predict(learner_typeid, (testset.x)[i]);
    if (predictedvalue != testset.y[i]) {
      err_ct++;
    }
    // save predicted values to file
    fprintf(fp, "%f\n", predictedvalue);
  }
  fclose(fp);
  /* calculate testing error */
  printf( "\n*** %d out of %d misclassified ***\n", err_ct, testset.n_points );
  printf( "*** Testing error is %f, accuracy is %f. ***\n", double(err_ct)/double(testset.n_points), 1- double(err_ct)/double(testset.n_points) );
  //fprintf( stderr, "*** Results are save in \"%s\" ***\n\n", predictedvalue_filename.c_str());
}

/**
* Load models from a file, and perform offline batch classification for multiple testing vectors
*
* @param: type id of the learner
* @param: testing set
* @param: name of the model file
* @param: name of the file to store classified labels
*/
template<typename TKernel>
void SVM<TKernel>::LoadModelBatchPredict(int learner_typeid, Dataset_sl& testset, String model_filename, String predictedvalue_filename) {
  LoadModel_(learner_typeid, model_filename);
  BatchPredict(learner_typeid, testset, predictedvalue_filename);
}


/**
* Save SVM model to a text file
*
* @param: type id of the learner
* @param: name of the model file
*/
// TODO: use XML
template<typename TKernel>
void SVM<TKernel>::SaveModel_(int learner_typeid, String model_filename) {
  FILE *fp = fopen(model_filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "Cannot save trained model to file!");
    return;
  }
  size_t i, j;

  if (learner_typeid == 0) { // for SVM_C
    fprintf(fp, "svm_type SVM_C\n");
    fprintf(fp, "total_num_sv %d\n", total_num_sv_);
    fprintf(fp, "num_classes %d\n", num_classes_);
    // save labels
    fprintf(fp, "labels ");
    for (i = 0; i < num_classes_; i++) 
      fprintf(fp, "%f ", train_labels_list_[i]);
    fprintf(fp, "\n");
    // save support vector info
    fprintf(fp, "sv_list_startpos ");
    for (i =0; i < num_classes_; i++)
      fprintf(fp, "%d ", sv_list_startpos_[i]);
    fprintf(fp, "\n");
    fprintf(fp, "sv_list_ct ");
    for (i =0; i < num_classes_; i++)
      fprintf(fp, "%d ", sv_list_ct_[i]);
    fprintf(fp, "\n");
  }
  else if (learner_typeid == 1) { // for SVM_R
    fprintf(fp, "svm_type SVM_R\n");
    fprintf(fp, "total_num_sv %d\n", total_num_sv_);
    fprintf(fp, "sv_index ");
    for (i = 0; i < total_num_sv_; i++)
      fprintf(fp, "%d ", sv_index_[i]);
    fprintf(fp, "\n");
  }
  else if (learner_typeid == 2) { // for SVM_Q
    fprintf(fp, "svm_type SVM_Q\n");
    fprintf(fp, "total_num_sv %d\n", total_num_sv_);
    fprintf(fp, "sv_index ");
    for (i = 0; i < total_num_sv_; i++)
      fprintf(fp, "%d ", sv_index_[i]);
    fprintf(fp, "\n");
  }

  // save kernel parameters
  fprintf(fp, "kernel_name %s\n", param_.kernelname_.c_str());
  fprintf(fp, "kernel_typeid %d\n", param_.kerneltypeid_);
  param_.kernel_.SaveParam(fp);

  // save models: bias, coefficients and support vectors
  fprintf(fp, "bias ");
  for (i = 0; i < num_models_; i++)
    fprintf(fp, "%.16g ", models_[i].bias_);
  fprintf(fp, "\n");
  
  fprintf(fp, "SV_coefs\n");
  for (i = 0; i < total_num_sv_; i++) {
    for (j = 0; j < num_classes_-1; j++) {
      fprintf(fp, "%.16g ", sv_coef_.get(j,i));
    }
    const struct NZ_entry *sv_p = sv_entries_[i];
    while (sv_p->index != -1) {
      fprintf(fp, "%d:%.8g ", sv_p->index+1, sv_p->value); // in svmlight's data format, feature index begins from 1, not 0
      sv_p ++;
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
}

/**
* Load SVM model file
*
* @param: type id of the learner
* @param: name of the model file
*/
// TODO: use XML
template<typename TKernel>
void SVM<TKernel>::LoadModel_(int learner_typeid, String model_filename) {
  if (learner_typeid == 0) {// SVM_C
    train_labels_list_.Renew();
    train_labels_list_.Init(num_classes_); // get labels list from the model file
  }

  /* load model file */
  FILE *fp = fopen(model_filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open SVM model file!");
    return;
  }
  char cmd[80]; 
  int i, j; int temp_d; double temp_f;
  for (i = 0; i < num_models_; i++) {
	models_.PushBack();
	models_[i].coef_.Init();
  }
  while (1) {
    fscanf(fp,"%80s",cmd);
    if(strcmp(cmd,"svm_type")==0) {
      fscanf(fp,"%80s", cmd);
      if (strcmp(cmd,"SVM_C")==0)
	learner_typeid_ = 0;
      else if (strcmp(cmd,"SVM_R")==0)
	learner_typeid_ = 1;
      else if (strcmp(cmd,"SVM_Q")==0)
	learner_typeid_ = 2;
    }
    else if (strcmp(cmd, "total_num_sv")==0) {
      fscanf(fp,"%d",&total_num_sv_);
    }
    // for SVM_C
    else if (strcmp(cmd, "num_classes")==0) {
      fscanf(fp,"%d",&num_classes_);
    }
    else if (strcmp(cmd, "labels")==0) {
      for (i=0; i<num_classes_; i++) {
	fscanf(fp,"%lf",&temp_f);
	train_labels_list_[i] = temp_f;
      }
    }
    else if (strcmp(cmd, "sv_list_startpos")==0) {
      for ( i= 0; i < num_classes_; i++) {
	fscanf(fp,"%d",&temp_d);
	sv_list_startpos_[i]= temp_d;
      }
    }
    else if (strcmp(cmd, "sv_list_ct")==0) {
      for ( i= 0; i < num_classes_; i++) {
	fscanf(fp,"%d",&temp_d); 
	sv_list_ct_[i]= temp_d;
      }
    }
    // for SVM_R
    else if (strcmp(cmd, "sv_index")==0) {
      for ( i= 0; i < total_num_sv_; i++) {
	fscanf(fp,"%d",&temp_d); 
	sv_index_.PushBack() = temp_d;
      }
    }
    // load kernel info
    else if (strcmp(cmd, "kernel_name")==0) {
      fscanf(fp,"%80s",param_.kernelname_.c_str());
    }
    else if (strcmp(cmd, "kernel_typeid")==0) {
      fscanf(fp,"%d",&param_.kerneltypeid_);
    }
    else if (strcmp(cmd, "sigma")==0) {
      fscanf(fp,"%lf",&param_.kernel_.kpara_[0]); /* for gaussian kernels only */
    }
    else if (strcmp(cmd, "gamma")==0) {
      fscanf(fp,"%lf",&param_.kernel_.kpara_[1]); /* for gaussian kernels only */
    }
    // load bias
    else if (strcmp(cmd, "bias")==0) {
      for ( i= 0; i < num_models_; i++) {
	fscanf(fp,"%lf",&temp_f); 
	models_[i].bias_= temp_f;
      }
    }
    else if (strcmp(cmd,"SV_coefs")==0) {
      while (1) {
	int c = getc(fp);
	if(c==EOF || c=='\n') {
	  break;
	}
      }
      break;
    }
  }

  // load coefficients and support vectors
  size_t num_nz_entries  = 0; // also include the indicators with index==-1
  long pos = ftell(fp);
  
  line_ = Malloc(char,max_line_length_);
  char *p, *endptr, *idx, *val;
  
  while(ReadLine(fp)!=NULL) {
    p = strtok(line_,":");
    while(1) {
      p = strtok(NULL,":");
      if(p == NULL)
	break;
      ++num_nz_entries;
    }
  }
  num_nz_entries += total_num_sv_; // for the last indicator "-1" of each point
  
  fseek(fp, pos, SEEK_SET);

  sv_coef_.Init(num_classes_-1, total_num_sv_);
  sv_coef_.SetZero();

  sv_entries_ = Malloc(NZ_entry*, total_num_sv_);
  sv_nz_pool_ = Malloc(NZ_entry, num_nz_entries);

  j = 0;
  for (i=0; i<total_num_sv_; i++) {
    ReadLine(fp);
    sv_entries_[i] = &sv_nz_pool_[j];

    p = strtok(line_, " \t");
    sv_coef_.set(0, i, strtod(p, &endptr));
    for (size_t k=1; k<num_classes_-1; k++) {
      p = strtok(NULL, " \t");
      sv_coef_.set(k, i, strtod(p, &endptr));
    }
    while (1) {
      idx = strtok(NULL, ":");
      val = strtok(NULL, " \t");
      if (val == NULL) {
	break;
      }
      sv_nz_pool_[j].index = (size_t) strtol(idx, &endptr, 10) - 1;
      sv_nz_pool_[j].value = strtod(val, &endptr);
      ++j;
    }
    sv_nz_pool_[j++].index = -1; // indicator of the end of a SV
  }
  free(line_);

  fclose(fp);

}


#endif
