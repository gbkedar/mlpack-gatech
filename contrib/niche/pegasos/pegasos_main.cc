/** @file pegasos_main.cc
 *
 *  Driver file for testing Pegasos
 *
 *  @author Nishant Mehta (niche)
 */

#include <fastlib/fastlib.h>
#include <armadillo>

#include "pegasos.h"



using namespace arma;
using namespace std;



int main(int argc, char* argv[]) {
  fx_module* root = fx_init(argc, argv, NULL);

  double lambda = fx_param_double_req(NULL, "lambda");

  // Note: fx_param_double lets us input via scientific notation
  u32 n_iterations = (u32) fx_param_double_req(NULL, "n_iterations");
  printf("n_iterations = %f\n", ((double)n_iterations));
  u32 mini_batch_size = fx_param_int(NULL, "k", 1);
  
  const char* data_fullpath = 
    fx_param_str_req(NULL, "data");
  const char* labels_fullpath = 
    fx_param_str_req(NULL, "labels");
  
  
  mat X;
  vec y;
  
  X.load(data_fullpath);
  y.load(labels_fullpath);
  
  
  Pegasos pegasos;
  pegasos.Init(X, y, lambda, n_iterations, mini_batch_size);
  
  pegasos.DoPegasos();
  
  
  vec w = pegasos.GetW();
  
  w.print("w");
  
  u32 n_errors = 0;
  for(u32 i = 0; i < X.n_cols; i++) {
    if(y(i) * dot(w, X.col(i)) <= 0) {
      n_errors++;
    }
  }
  
  printf("%d misclassifications\n", n_errors);
  
  printf("%f%% error\n", 100.0 * ((double)n_errors) / ((double)X.n_cols));
      
      
  

  fx_done(root);

}
  
