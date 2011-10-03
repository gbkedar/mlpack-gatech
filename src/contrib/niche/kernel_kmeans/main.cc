#include "fastlib/fastlib.h"
#include "kernel_kmeans.h"
#include "mlpack/fastica/lin_alg.h"

const fx_entry_doc kernel_kmeans_main_entries[] = {
  FX_ENTRY_DOC_DONE
};

const fx_submodule_doc kernel_kmeans_main_submodules[] = {
  FX_SUBMODULE_DOC_DONE
};

const fx_module_doc kernel_kmeans_main_doc = {
  kernel_kmeans_main_entries, kernel_kmeans_main_submodules,
  "This program implements the kernel k-means algorithm in Dhillon, Guan, and Kulis (2004).\n"
};


int main(int argc, char *argv[]) {
  fx_module* root = fx_init(argc, argv, &kernel_kmeans_main_doc);


  srand48(time(0));
  srand(time(0));
  

  /*
  Matrix kernel_matrix;

  int n_points = 10;
  int n_dims = 3;

  kernel_matrix.Init(n_points, n_points);
  kernel_matrix.SetAll(-1);
  */
  /*  
  ArrayList<Vector> points;
  points.Init(n_points);
  for(int i = 0; i < n_points; i++) {
    points[i].Init(n_dims);
    linalg__private::RandVector(points[i]);
  }


  
  Vector diff;
  diff.Init(n_dims);
  for(int i = 0; i < n_points; i++) {
    for(int j = 0; j < n_points; j++) {
      la::SubOverwrite(points[i], points[j], &diff);
      kernel_matrix.set(i, j, exp(-1 * la::Dot(diff, diff)));
    }
  }
  */

  const char* kernel_matrix_filename = fx_param_str_req(NULL, "k_filename");

  Matrix kernel_matrix;
  data::Load(kernel_matrix_filename, &kernel_matrix);
  //la::Scale(1000, &kernel_matrix);

  int n_points;
  n_points = kernel_matrix.n_cols();

  Vector weights;
  weights.Init(n_points);
  weights.SetAll(1);

  int k = 2;

  KernelKmeans kernel_kmeans;
  kernel_kmeans.Init(&kernel_matrix, &weights, k);

  int cluster_memberships[n_points];
  kernel_kmeans.Compute(cluster_memberships);

  printf("cluster memberships\n");
  for(int i = 0; i < n_points; i++) {
    printf("%d\n", cluster_memberships[i]);
  }

  int n_matches = 0;
  for(int i = 0; i < (n_points / 2); i++) {
    n_matches += (cluster_memberships[i] == 1);
  }
  for(int i = (n_points / 2); i < n_points; i++) {
    n_matches += (cluster_memberships[i] == 0);
  }

  if(n_matches < (n_points / 2)) {
    n_matches = n_points - n_matches;
  }

  printf("%d correct cluster assignments out of %d points\n",
	 n_matches, n_points);
  printf("Accuracy: %f\n", ((double)n_matches) / ((double)n_points));
    
    
  
  fx_done(root);

  return true;
}


  

  


