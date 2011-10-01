/**
 * @file main.cc
 * @ Parikshit Ram (pram@cc.gatech.edu)
 *
 */

#include <string>

#include "fastlib/fastlib.h"
#include "dtree.h"

const fx_entry_doc dtree_main_entries[] = {
  {"data", FX_REQUIRED, FX_STR, NULL,
   " Data file \n"},
  {"test", FX_PARAM, FX_STR, NULL,
   "The points at which the density is to be computed"
   " using the tree.\n"},
  {"test_output", FX_PARAM, FX_STR, NULL,
   "File in which the density at the test points is "
   "to be output.\n"},
  {"folds", FX_PARAM, FX_INT, NULL,
   " Number of folds for cross validation. For LOOCV "
   "enter 0.\n"},
  {"tree_file", FX_PARAM, FX_STR, NULL,
   " The file in which the tree would be printed.\n"},
  {"train_unpruned_output", FX_PARAM, FX_STR, NULL,
   " The file in which the estimated density values at"
   " the training points are output for the unpruned tree.\n"},
  {"train_output", FX_PARAM, FX_STR, NULL,
   " The file in which the estimated density values at"
   " the training points are output.\n"},
  {"train_time", FX_TIMER, FX_CUSTOM, NULL,
   " Training time for obtaining the optimal tree.\n"},
  {"test_time", FX_TIMER, FX_CUSTOM, NULL,
   " Testing time for the optimal decision tree.\n"},
  {"print_tree", FX_PARAM, FX_BOOL, NULL,
   " Whether to print the tree or not.\n"},
  FX_ENTRY_DOC_DONE
};

const fx_submodule_doc dtree_main_submodules[] = {
  {"dtree", &dtree_doc,
   " Contains the parameters for growing the tree.\n"},
//   {"dt_utils", &dt_utils_doc,
//    " Utility functions for performing density estimation "
//    "using decision trees.\n"},
  FX_SUBMODULE_DOC_DONE
};

const fx_module_doc dtree_main_doc = {
  dtree_main_entries, dtree_submodules,
  "DTree Parameters \n"
};

void PermuteMatrix(const Matrix&, Matrix*);
void DoFunkyStuff(DTree *dtree, const Matrix& data,
		  const Matrix& labels, size_t num_classes);


int main(int argc, char *argv[]){

  srand( time(NULL));
  fx_module *root = fx_init(argc, argv, &dtree_main_doc);
  std::string data_file = fx_param_str_req(root, "d");
  Matrix dataset;
  NOTIFY("Loading data file...\n");
  data::Load(data_file.c_str(), &dataset);

  NOTIFY("%zud points in %zud dims.", dataset.n_cols(),
	 dataset.n_rows());

//   // getting information about the feature type
//   // REAL, INTEGER, NOMINAL. Most probably using
//   // enum ... don't know how to use it though.
//   ArrayList<enum> dim_type;
//   dim_type.Init(dataset.n_rows());
//   for (size_t i = 0; i < dim_type.size(); i++) {
//     // assign dim type somehow
//   } // end for

  // finding the max and min vals for the dataset
  ArrayList<double> max_vals, min_vals;
  max_vals.Init(dataset.n_rows());
  min_vals.Init(dataset.n_rows());

  Matrix temp_d;
  la::TransposeInit(dataset, &temp_d);

  for (size_t i = 0; i < temp_d.n_cols(); i++) {
    // if (dim_type[i] != NOMINAL) {
    Vector dim_vals;
    temp_d.MakeColumnVector(i, &dim_vals);
    std::vector<double> dim_vals_vec(dim_vals.ptr(),
				     dim_vals.ptr() + temp_d.n_rows());

    sort(dim_vals_vec.begin(), dim_vals_vec.end());
    min_vals[i] = *(dim_vals_vec.begin());
    max_vals[i] = *(dim_vals_vec.end() -1);
    // }
  }

  // Initializing the tree
  DTree *dtree = new DTree();
  dtree->Init(max_vals, min_vals, dataset.n_cols());
  // Getting ready to grow the tree
  ArrayList<size_t> old_from_new;
  old_from_new.Init(dataset.n_cols());
  for (size_t i = 0; i < old_from_new.size(); i++) {
    old_from_new[i] = i;
  }

  // Saving the dataset since it would be modified
  // while growing the tree
  Matrix new_dataset;
  new_dataset.Copy(dataset);

  // starting the training timer
  fx_timer_start(root, "train_time");
  // Growing the tree
  long double old_alpha = 0.0;
  long double alpha = dtree->Grow(new_dataset, &old_from_new);
  double new_f = dtree->st_estimate();
  double old_f = new_f;

  NOTIFY("%zud leaf nodes in this tree, min_alpha: %Lg",
	 dtree->subtree_leaves(), alpha);

  // computing densities for the train points in the
  // full tree if asked for.
  if (fx_param_exists(root, "train_full_tree_density_file")) {
    FILE *fp = 
      fopen(fx_param_str_req(root, "train_full_tree_density_file"), "w");
    if (fp != NULL) {
      for (size_t i = 0; i < dataset.n_cols(); i++) {
	Vector test_p;
	dataset.MakeColumnVector(i, &test_p);
	double f = dtree->ComputeValue(test_p, false);
	fprintf(fp, "%lg\n", f);
      } // end for
      fclose(fp);
    }
  }

  // exit(0);
  // sequential pruning and saving the alpha vals and the
  // values of c_t^2*r_t
  std::vector<std::pair<long double, long double> >  pruned_sequence;
  std::vector<double> change_in_estimate;
  while (dtree->subtree_leaves() > 1) {
    std::pair<long double, long double> tree_seq (old_alpha,
					-1.0 * dtree->subtree_leaves_error());
    pruned_sequence.push_back(tree_seq);
    change_in_estimate.push_back(fabs(new_f - old_f));
    old_alpha = alpha;
    old_f = new_f;
    alpha = dtree->PruneAndUpdate(old_alpha);
    new_f = dtree->st_estimate();
    DEBUG_ASSERT_MSG((alpha < LDBL_MAX)||(dtree->subtree_leaves() == 1),
		     "old_alpha:%Lg, alpha:%Lg, tree size:%zud",
		     old_alpha, alpha, dtree->subtree_leaves());
    DEBUG_ASSERT(alpha > old_alpha);
  } // end while
  std::pair<long double, long double> tree_seq (old_alpha,
                                  -1.0 * dtree->subtree_leaves_error());
  pruned_sequence.push_back(tree_seq);
  change_in_estimate.push_back(fabs(new_f - old_f));

  NOTIFY("%zud trees in the sequence, max_alpha:%Lg.\n",
	 (size_t) pruned_sequence.size(), old_alpha);


  // exit(0);

  // cross-validation here
  size_t folds = fx_param_int(root, "folds", 10);
  if (folds == 0) {
    folds = dataset.n_cols();
    NOTIFY("Starting Leave-One-Out Cross validation");
  } else 
    NOTIFY("Starting %zud-fold Cross validation", folds);

  // Permute the dataset once just for keeps
  Matrix pdata;
  //  PermuteMatrix(dataset, &pdata);
  pdata.Copy(dataset);
  //  pdata.PrintDebug("Per");
  size_t test_size = dataset.n_cols() / folds;

  // Go through each fold
  for (size_t fold = 0; fold < folds; fold++) {
    // NOTIFY("Fold %zud...", fold+1);

    // break up data into train and test set
    Matrix test;
    size_t start = fold * test_size,
      end = min ((fold + 1) * test_size,dataset.n_cols());
    pdata.MakeColumnSlice(start, end - start, &test);
    Matrix train;
    train.Init(pdata.n_rows(), pdata.n_cols() - (end - start));
    size_t k = 0;
    for (size_t j = 0; j < pdata.n_cols(); j++) {
      if (j < start || j >= end) {
	Vector temp_vec;
	pdata.MakeColumnVector(j, &temp_vec);
	train.CopyVectorToColumn(k++, temp_vec);
      } // end if
    } // end for
    DEBUG_ASSERT(k == train.n_cols());

    // go through the motions - computing 
    // the maximum and minimum for each dimensions
    ArrayList<double> max_vals_cv, min_vals_cv;
    max_vals_cv.Init(train.n_rows());
    min_vals_cv.Init(train.n_rows());

    Matrix temp_t;
    la::TransposeInit(train, &temp_t);

    for (size_t i = 0; i < temp_t.n_cols(); i++) {
      Vector dim_vals;
      temp_t.MakeColumnVector(i, &dim_vals);

      std::vector<double> dim_vals_vec(dim_vals.ptr(),
				       dim_vals.ptr()
				       + temp_t.n_rows());
      sort(dim_vals_vec.begin(), dim_vals_vec.end());
      min_vals_cv[i] = *(dim_vals_vec.begin());
      max_vals_cv[i] = *(dim_vals_vec.end() -1);
    } // end for

    // Initializing the tree
    DTree *dtree_cv = new DTree();
    dtree_cv->Init(max_vals_cv, min_vals_cv, train.n_cols());
    // Getting ready to grow the tree
    ArrayList<size_t> old_from_new_cv;
    old_from_new_cv.Init(train.n_cols());
    for (size_t i = 0; i < old_from_new_cv.size(); i++) {
      old_from_new_cv[i] = i;
    }

    // Growing the tree
    old_alpha = 0.0;
    alpha = dtree_cv->Grow(train, &old_from_new_cv);

    // sequential pruning with all the values of available
    // alphas and adding values for test values
    std::vector<std::pair<long double, long double> >::iterator it;
    for (it = pruned_sequence.begin();
	 it < pruned_sequence.end() -2; ++it) {
      
      // compute test values for this state of the tree
      long double val_cv = 0.0;
      for (size_t i = 0; i < test.n_cols(); i++) {
	Vector test_point;
	test.MakeColumnVector(i, &test_point);
	val_cv += dtree_cv->ComputeValue(test_point, false);
      }

      // update the cv error value
      it->second = it->second - 2.0 * val_cv / (double) dataset.n_cols();

      // getting the new alpha value and pruning accordingly
      old_alpha = sqrt(((it+1)->first) * ((it+2)->first));
      alpha = dtree_cv->PruneAndUpdate(old_alpha);
    } // end for

    // compute test values for this state of the tree
    long double val_cv = 0.0;
    for (size_t i = 0; i < test.n_cols(); i++) {
      Vector test_point;
      test.MakeColumnVector(i, &test_point);
      val_cv += dtree_cv->ComputeValue(test_point, false);
    }
    // update the cv error value
    it->second = it->second - 2.0 * val_cv / (long double) dataset.n_cols();

  } // end for

  long double optimal_alpha = -1.0, best_cv_error = DBL_MAX;
  std::vector<std::pair<long double, long double> >::iterator it;
  std::vector<double>::iterator jt;
  for (it = pruned_sequence.begin(), jt = change_in_estimate.begin();
       it < pruned_sequence.end() -1; ++it, ++jt) {
    if (it->second < best_cv_error) {
      best_cv_error = it->second;
      optimal_alpha = it->first;
    } // end if
//     printf("%Lg,%Lg,%lg\n", it->first, it->second, *jt);
  } // end for


  // Initializing the tree
  DTree *dtree_opt = new DTree();
  dtree_opt->Init(max_vals, min_vals, dataset.n_cols());
  // Getting ready to grow the tree
  for (size_t i = 0; i < old_from_new.size(); i++) {
    old_from_new[i] = i;
  }

  // Saving the dataset since it would be modified
  // while growing the tree
  new_dataset.Destruct();
  new_dataset.Copy(dataset);

  // Growing the tree
  old_alpha = 0.0;
  alpha = dtree_opt->Grow(new_dataset, &old_from_new);
  NOTIFY("%zud leaf nodes in this tree\n opt_alpha:%Lg",
	 dtree_opt->subtree_leaves(), optimal_alpha);
//   printf("%zud leaf nodes in this tree\n opt_alpha:%Lg\n",
// 	 dtree_opt->subtree_leaves(), optimal_alpha);

  // BS hack for optimal alpha
  // optimal_alpha /= 10;

  // Pruning with optimal alpha
  while (old_alpha < optimal_alpha) {
    old_alpha = alpha;
    alpha = dtree_opt->PruneAndUpdate(old_alpha);
    DEBUG_ASSERT_MSG((alpha < DBL_MAX)||(dtree->subtree_leaves() == 1),
		     "old_alpha:%Lg, alpha:%Lg, tree size:%zud",
		     old_alpha, alpha, dtree->subtree_leaves());
    DEBUG_ASSERT(alpha > old_alpha);
  } // end while

  NOTIFY("%zud leaf nodes in this tree\n old_alpha:%Lg",
	 dtree_opt->subtree_leaves(), old_alpha);

  // stopping the training timer
  fx_timer_stop(root, "train_time");

  if (fx_param_bool(root, "print_tree", false)) {
    dtree_opt->WriteTree(0);
    printf("\n");fflush(NULL);
  }
  
  // starting the test timer
  fx_timer_start(root, "test_time");


  // computing densities for the train points in the
  // optimal tree
  FILE *fp;
  fp = NULL;
  if (fx_param_exists(root, "train_density_file")) {
    std::string train_density_file
      = fx_param_str_req(root, "train_density_file");
    fp = fopen(train_density_file.c_str(), "w");
  }
  if (fx_param_bool(root, "compute_training", 0)) {
    for (size_t i = 0; i < dataset.n_cols(); i++) {
      Vector test_p;
      dataset.MakeColumnVector(i, &test_p);
      double f = dtree_opt->ComputeValue(test_p, true);
      if (fp != NULL) {
	fprintf(fp, "%lg\n", f);
      }
    } // end for
  }

  if (fp != NULL)
    fclose(fp);


  // computing the density at the provided test points
  // and outputting the density in the given file.
  if (fx_param_exists(root, "test_points")) {
    std::string test_file = fx_param_str_req(root, "test_points");
    Matrix test_set;
    NOTIFY("Loading test data...\n");
    data::Load(test_file.c_str(), &test_set);

    NOTIFY("%zud points in %zud dims.", test_set.n_cols(),
	   test_set.n_rows());

    std::string test_density_file
      = fx_param_str_req(root, "test_density_file");
    FILE *fp = fopen(test_density_file.c_str(), "w");
    if (fp != NULL) {
      for (size_t i = 0; i < test_set.n_cols(); i++) {
	Vector test_p;
	test_set.MakeColumnVector(i, &test_p);
	double f = dtree_opt->ComputeValue(test_p, false);
	fprintf(fp, "%lg\n", f);
      } // end for
    }
    fclose(fp);
  }
  fx_timer_stop(root, "test_time");

//   // outputting the optimal tree
//   std::string output_file
//     = fx_param_str(root, "tree_file", "output.txt");

  if (fx_param_exists(root, "labels")) {
    std::string labels_file = fx_param_str_req(root, "labels");
    Matrix labels;
    NOTIFY("loading labels.\n");
    data::Load(labels_file.c_str(), &labels);
    size_t num_classes = fx_param_int_req(root, "num_classes");

    DEBUG_ASSERT(dataset.n_cols() == labels.n_cols());
    DEBUG_ASSERT(labels.n_rows() == 1);

    DoFunkyStuff(dtree_opt, dataset, labels, num_classes);
  }

  if(fx_param_bool(root, "print_vi", false)) {
    ArrayList<long double> *imps = new ArrayList<long double>();
    imps->Init(dataset.n_rows());
    for (size_t i = 0; i < imps->size(); i++) {
      (*imps)[i] = 0.0;
    }

    dtree_opt->ComputeVariableImportance(imps);
    long double max = 0.0;
    for (size_t i = 0; i < imps->size(); i++)
      if ((*imps)[i] > max)
	max = (*imps)[i];
    printf("Max: %Lg\n", max); fflush(NULL);

    for (size_t i = 0; i < imps->size(); i++)
      printf("%Lg,", (*imps)[i]);

    printf("\b\n ------------------------------- \n\n");fflush(NULL);

    for (size_t i = 0; i < imps->size(); i++)
      if ((*imps)[i] > 0.0) 
	printf("256,");
      else
	printf("0,");

    printf("\b\n");fflush(NULL);
  }

  fx_param_bool(root, "fx/silent", 0);
  fx_done(root);
}

void PermuteMatrix(const Matrix& input, Matrix *output) {

  ArrayList<size_t> perm_array;
  size_t size = input.n_cols();
  Matrix perm_mat;
  perm_mat.Init(size, size);
  perm_mat.SetAll(0.0);
  srand( time(NULL));
  math::MakeRandomPermutation(size, &perm_array);
  for(size_t i = 0; i < size; i++) {
    perm_mat.set(perm_array[i], i, 1.0);
  }
  la::MulInit(input, perm_mat, output);
  return;
}

 void DoFunkyStuff(DTree *dtree, const Matrix& data,
		   const Matrix& labels, size_t num_classes) {

   size_t num_leaves = dtree->TagTree(0);
   dtree->WriteTree(0);printf("\n");fflush(NULL);
   Matrix table;
   table.Init(num_leaves, num_classes);
   table.SetZero();
   for (size_t i = 0; i < data.n_cols(); i++) {
     Vector test_p;
     data.MakeColumnVector(i, &test_p);
     size_t leaf_tag = dtree->FindBucket(test_p);
     size_t label = (size_t) labels.get(0, i);
//      printf("%zud,%zud - %lg\n", leaf_tag, label, 
// 	    table.get(leaf_tag, label));
     table.set(leaf_tag, label, 
	       table.get(leaf_tag, label) + 1.0);
   } // end for

   table.PrintDebug("Classes in each leaf");


   return;

   // maybe print some more statistics if these work out well
 }
