/** @file dualtree_vkde.h
 *
 *  This file contains an implementation of the variable-bandwidth
 *  kernel density estimation for a linkable library component. It
 *  implements a rudimentary depth-first dual-tree algorithm with
 *  finite difference, using the formalized GNP framework by Ryan and
 *  Garry.
 *
 *  For more details on mathematical derivations, please take a look at
 *  the published conference papers (in chronological order):
 *
 *  inproceedings{DBLP:conf/sdm/GrayM03,
 *   author    = {Alexander G. Gray and Andrew W. Moore},
 *   title     = {Nonparametric Density Estimation: Toward Computational 
 *                Tractability},
 *   booktitle = {SDM},
 *   year      = {2003},
 *   ee        = {http://www.siam.org/meetings/sdm03/proceedings/sdm03_19.pdf},
 *   crossref  = {DBLP:conf/sdm/2003},
 *   bibsource = {DBLP, http://dblp.uni-trier.de}
 *  }
 *
 *  misc{ gray03rapid,
 *   author = "A. Gray and A. Moore",
 *   title = "Rapid evaluation of multiple density models",
 *   booktitle = "In C. M. Bishop and B. J. Frey, editors, 
 *                Proceedings of the Ninth International Workshop on 
 *                Artificial Intelligence and Statistics",
 *   year = "2003",
 *   url = "citeseer.ist.psu.edu/gray03rapid.html"
 *  }
 *
 *  incollection{NIPS2005_570,
 *   title = {Dual-Tree Fast Gauss Transforms},
 *   author = {Dongryeol Lee and Alexander Gray and Andrew Moore},
 *   booktitle = {Advances in Neural Information Processing Systems 18},
 *   editor = {Y. Weiss and B. Sch\"{o}lkopf and J. Platt},
 *   publisher = {MIT Press},
 *   address = {Cambridge, MA},
 *   pages = {747--754},
 *   year = {2006}
 *  }
 *
 *  inproceedings{DBLP:conf/uai/LeeG06,
 *   author    = {Dongryeol Lee and Alexander G. Gray},
 *   title     = {Faster Gaussian Summation: Theory and Experiment},
 *   booktitle = {UAI},
 *   year      = {2006},
 *   crossref  = {DBLP:conf/uai/2006},
 *   bibsource = {DBLP, http://dblp.uni-trier.de}
 *  }
 *
 *  @author Dongryeol Lee (dongryel)
 *  @see kde_main.cc
 *  @bug No known bugs.
 */

#ifndef DUALTREE_VKDE_H
#define DUALTREE_VKDE_H

#define INSIDE_DUALTREE_VKDE_H

#include "mlpack/core.h"
#include "mlpack/core/tree/binary_space_tree.hpp"
//#include "contrib/dongryel/proximity_project/gen_metric_tree.h"
#include "dualtree_kde_common.h"
//#include "kde_stat.h"
#include "mlpack/methods/neighbor_search/neighbor_search.h"

/** @brief A computation class for dual-tree based variable-bandwidth
 *         kernel density estimation.
 *
 *  This class builds trees for input query and reference sets on Init.
 *  The KDE computation is then performed by calling Compute.
 *
 *  This class is only intended to compute once per instantiation.
 *
 *  Example use:
 *
 *  @code
 *    DualtreeVKde fast_kde;
 *    struct datanode* kde_module;
 *    arma::vec results;
 *
 *    kde_module = fx_submodule(NULL, "kde", "kde_module");
 *    fast_kde.Init(queries, references, queries_equal_references,
 *                  kde_module);
 *
 *    // important to make sure that you don't call Init on results!
 *    fast_kde.Compute(&results);
 *  @endcode
 */
template<typename TKernel>
class DualtreeVKde {
  
  friend class DualtreeKdeCommon;

 public:
  
  // our tree type using the VKdeStat
  typedef tree::BinarySpaceTree<DBallBound < kernel::LMetric<2>, arma::vec>, VKdeStat<TKernel> > Tree;
    
 private:

  ////////// Private Constants //////////

  /** @brief The number of initial samples to take per each query when
   *         doing Monte Carlo sampling.
   */
  static const int num_initial_samples_per_query_ = 25;

  static const int sample_multiple_ = 10;

  ////////// Private Member Variables //////////

  /** @brief The pointer to the module holding the parameters.
   */
  struct datanode *module_;

  /** @brief The boolean flag to control the leave-one-out computation.
   */
  bool leave_one_out_;

  /** @brief The normalization constant.
   */
  double mult_const_;

  /** @brief The kernel objects, one for each reference point.
   */
  std::vector<TKernel> kernels_;

  /** @brief The query dataset.
   */
  arma::mat qset_;

  /** @brief The query tree.
   */
  Tree *qroot_;

  /** @brief The reference dataset.
   */
  arma::mat rset_;
  
  /** @brief The reference tree.
   */
  Tree *rroot_;

  /** @brief The reference weights.
   */
  arma::vec rset_weights_;

  /** @brief The running lower bound on the densities.
   */
  arma::vec densities_l_;

  /** @brief The computed densities.
   */
  arma::vec densities_e_;

  /** @brief The running upper bound on the densities.
   */
  arma::vec densities_u_;

  /** @brief The amount of used error for each query.
   */
  arma::vec used_error_;

  /** @brief The number of reference points taken care of for each
   *         query.
   */
  arma::vec n_pruned_;

  /** @brief The sum of all reference weights.
   */
  double rset_weight_sum_;

  /** @brief The accuracy parameter specifying the relative error
   *         bound.
   */
  double relative_error_;

  /** @brief The accuracy parameter: if the true sum is less than this
   *         value, then relative error is not guaranteed. Instead the
   *         sum is guaranteed an absolute error bound.
   */
  double threshold_;
  
  /** @brief The number of finite difference prunes.
   */
  int num_finite_difference_prunes_;

  /** @brief The number of prunes using Monte Carlo.
   */
  int num_monte_carlo_prunes_;

  /** @brief The permutation mapping indices of queries_ to original
   *         order.
   */
  std::vector<size_t> old_from_new_queries_;
  
  /** @brief The permutation mapping indices of references_ to
   *         original order.
   */
  std::vector<size_t> old_from_new_references_;

  ////////// Private Member Functions //////////

  /** @brief The exhaustive base KDE case.
   */
  void DualtreeVKdeBase_(Tree *qnode, Tree *rnode, double probability);

  /** @brief Checking for prunability of the query and the reference
   *         pair using four types of pruning methods.
   */
  bool PrunableEnhanced_(Tree *qnode, Tree *rnode, double probability,
			 Range &dsqd_range, Range &kernel_value_range, 
			 double &dl, double &du,
			 double &used_error, double &n_pruned,
			 int &order_farfield_to_local,
			 int &order_farfield, int &order_local);

  double EvalUnnormOnSq_(size_t reference_point_index,
			 double squared_distance);

  /** @brief Canonical dualtree KDE case.
   *
   *  @param qnode The query node.
   *  @param rnode The reference node.
   *  @param probability The required probability; 1 for exact
   *         approximation.
   *
   *  @return true if the entire contribution of rnode has been
   *          approximated using an exact method, false otherwise.
   */
  bool DualtreeVKdeCanonical_(Tree *qnode, Tree *rnode, double probability);

  /** @brief Pre-processing step - this wouldn't be necessary if the
   *         core fastlib supported a Init function for Stat objects
   *         that take more arguments.
   */
  void PreProcess(Tree *node, bool reference_side);

  /** @brief Post processing step.
   */
  void PostProcess(Tree *qnode);
    
 public:

  ////////// Constructor/Destructor //////////

  /** @brief The default constructor.
   */
  DualtreeVKde() {
    qroot_ = rroot_ = NULL;
  }

  /** @brief The default destructor which deletes the trees.
   */
  ~DualtreeVKde() { 
    
    if(qroot_ != rroot_ ) {
      delete qroot_; 
      delete rroot_; 
    } 
    else {
      delete rroot_;
    }

  }

  ////////// Getters and Setters //////////

  /** @brief Get the density estimate.
   */
  void get_density_estimates(arma::vec *results) { 
    *results = arma::vec(densities_e_.size());
    
    for(size_t i = 0; i < densities_e_.size(); i++) {
      (*results)[i] = densities_e_[i];
    }
  }

  ////////// User Level Functions //////////

  void Compute(arma::vec *results) {

    // Set accuracy parameters.
    relative_error_ = CLI::GetParam<double>("relative_error");
    threshold_ = CLI::GetParam<double>("threshold") *
      kernels_[0].Normalizer();//CalcNormConstant(qset_.n_rows);
    
    // initialize the lower and upper bound densities
    densities_l_.zeros();
    densities_e_.zeros();
    densities_u_.fill(rset_weight_sum_);

    // Set zero for error accounting stuff.
    used_error_.zeros();
    n_pruned_.zeros();

    // Reset prune statistics.
    num_finite_difference_prunes_ = num_monte_carlo_prunes_ = 0;

    printf("\nStarting variable KDE using %d neighbors...\n",
	   (int) CLI::GetParam<int>("knn"));

    CLI::StartTimer("fast_kde_compute");

    // Preprocessing step for initializing series expansion objects
    PreProcess(rroot_, true);
    if(qroot_ != rroot_) {
      PreProcess(qroot_, false);
    }
    
    // Get the required probability guarantee for each query and call
    // the main routine.
    double probability = CLI::GetParam<double>("probability");
    DualtreeVKdeCanonical_(qroot_, rroot_, probability);

    // Postprocessing step for finalizing the sums.
    PostProcess(qroot_);
    CLI::StopTimer("fast_kde_compute");
    printf("\nFast KDE completed...\n");
    printf("Finite difference prunes: %d\n", num_finite_difference_prunes_);
    printf("Monte Carlo prunes: %d\n", num_monte_carlo_prunes_);

    // Reshuffle the results to account for dataset reshuffling
    // resulted from tree constructions.
    arma::vec tmp_q_results(densities_e_.size());
    
    for(size_t i = 0; i < tmp_q_results.size(); i++) {
      tmp_q_results[old_from_new_queries_[i]] =
	densities_e_[i];
    }
    for(size_t i = 0; i < tmp_q_results.size(); i++) {
      densities_e_[i] = tmp_q_results[i];
    }

    // Retrieve density estimates.
    get_density_estimates(results);
  }

  void Init(const arma::mat &queries, const arma::mat &references,
	    const arma::mat &rset_weights, bool queries_equal_references, 
	    struct datanode *module_in) {

    // point to the incoming module
    module_ = module_in;

    // Set the flag for whether to perform leave-one-out computation.
    leave_one_out_ = CLI::HasParam("loo") &&
      (&queries == &references);

    // read in the number of points owned by a leaf
    //TODO int leaflen = CLI::GetParam<int>("leaflen");

    // Copy reference dataset and reference weights and compute its
    // sum. rset_weight_sum_ should be the raw sum of the reference
    // weights, ignoring the possibly different normalizing constants
    // in the case of variable-bandwidth case.
    rset_ = arma::mat(references.n_rows, references.n_cols);
    for (size_t c = 0; c < references.n_cols; ++c)
    {
      for (size_t r = 0; r < references.n_rows; ++r)
      {
        rset_(r,c) = references(r,c);
      }
    }
    rset_weights_ = arma::vec(rset_weights.n_cols);
    rset_weight_sum_ = 0;
    for(size_t i = 0; i < rset_weights.n_cols; i++) {
      rset_weights_[i] = rset_weights(0, i);
      rset_weight_sum_ += rset_weights_[i];
    }

    // Copy query dataset.
    if(queries_equal_references) {
      qset_ = rset_;
    }
    else {
      qset_ = arma::mat(queries.n_rows, queries.n_cols);
      for (size_t c = 0; c < queries.n_cols; ++c)
      {
        for (size_t r = 0; r < queries.n_rows; ++r)
        {
          qset_(r,c) = queries(r,c);
        }
      }
    }

    // Construct query and reference trees. Shuffle the reference
    // weights according to the permutation of the reference set in
    // the reference tree.
    CLI::StartTimer("tree_d");
    rroot_ = new Tree(rset_, /*leaflen,*/ old_from_new_references_);
    DualtreeKdeCommon::ShuffleAccordingToPermutation
      (rset_weights_, old_from_new_references_);

    if(queries_equal_references) {
      qroot_ = rroot_;
      old_from_new_queries_ = old_from_new_references_;
    }
    else {
      qroot_ = new Tree(qset_, /*leaflen*/ old_from_new_queries_);
    }
    CLI::StopTimer("tree_d");

    // Initialize the density lists
    densities_l_ = arma::vec(qset_.n_cols);
    densities_e_ = arma::vec(qset_.n_cols);
    densities_u_ = arma::vec(qset_.n_cols);

    // Initialize the error accounting stuff.
    used_error_ = arma::vec(qset_.n_cols);
    n_pruned_ = arma::vec(qset_.n_cols);

    // Initialize the kernels for each reference point.
    int knns = CLI::GetParam<int>("knn");
    AllkNN all_knn = AllkNN(rset_, knns);
    arma::Mat<size_t> resulting_neighbors;
    arma::mat squared_distances;

    CLI::StartTimer("bandwidth_initialization");
    all_knn.ComputeNeighbors(resulting_neighbors, squared_distances);

    for(size_t i = 0; i < squared_distances.size(); i += knns)
    {
      kernels_.push_back(kernel::GaussianKernel(sqrt(squared_distances[i + knns - 1])));
    }
    CLI::StopTimer("bandwidth_initialization");

    // Renormalize the reference weights according to the bandwidths
    // that have been chosen.
    double min_norm_const = DBL_MAX;
    for(size_t i = 0; i < rset_weights_.size(); i++) {
      double norm_const = kernels_[i].Normalizer();//CalcNormConstant(qset_.n_rows);
      min_norm_const = std::min(min_norm_const, norm_const);
    }
    for(size_t i = 0; i < rset_weights_.size(); i++) {
      double norm_const = kernels_[i].Normalizer();//CalcNormConstant(qset_.n_rows);
      rset_weights_[i] *= (min_norm_const / norm_const);
    }

    // Compute normalization constant.
    mult_const_ = 1.0 / min_norm_const;
  }

  void PrintDebug() {

    FILE *stream = stdout;
    const char *fname = NULL;

    if((fname = CLI::GetParam<std::string>("fast_kde_output").c_str()) != NULL)
    {
      stream = fopen(fname, "w+");
    }
    for(size_t q = 0; q < qset_.n_cols; q++) {
      fprintf(stream, "%g\n", densities_e_[q]);
    }

    if(stream != stdout) {
      fclose(stream);
    }
  }

};

#include "dualtree_vkde_impl.h"
#undef INSIDE_DUALTREE_VKDE_H

#endif
