/**
 * @file em_fit.hpp
 * @author Ryan Curtin
 *
 * Utility class to fit a GMM using the EM algorithm.  Used by
 * GMM::Estimate<>().
 */
#ifndef __MLPACK_METHODS_GMM_EM_FIT_HPP
#define __MLPACK_METHODS_GMM_EM_FIT_HPP

#include <mlpack/core.hpp>

// Default clustering mechanism.
#include <mlpack/methods/kmeans/kmeans.hpp>

namespace mlpack {
namespace gmm {

/**
 * This class contains methods which can fit a GMM to observations using the EM
 * algorithm.  It requires an initial clustering mechanism, which is by default
 * the KMeans algorithm.  The clustering mechanism must implement the following
 * method:
 *
 *  - void Cluster(const arma::mat& observations,
 *                 const size_t clusters,
 *                 arma::Col<size_t>& assignments);
 *
 * This method should create 'clusters' clusters, and return the assignment of
 * each point to a cluster.
 */
template<typename InitialClusteringType = kmeans::KMeans<> >
class EMFit
{
 public:
  /**
   * Construct the EMFit object, optionally passing an InitialClusteringType
   * object (just in case it needs to store state).  Setting the maximum number
   * of iterations to 0 means that the EM algorithm will iterate until
   * convergence (with the given tolerance).
   *
   * The parameter forcePositive controls whether or not the covariance matrices
   * are checked for positive definiteness at each iteration.  This could be a
   * time-consuming task, so, if you know your data is well-behaved, you can set
   * it to false and save some runtime.
   *
   * @param maxIterations Maximum number of iterations for EM.
   * @param tolerance Log-likelihood tolerance required for convergence.
   * @param forcePositive Check for positive-definiteness of each covariance
   *     matrix at each iteration.
   * @param clusterer Object which will perform the initial clustering.
   */
  EMFit(const size_t maxIterations = 300,
        const double tolerance = 1e-10,
        const bool forcePositive = true,
        InitialClusteringType clusterer = InitialClusteringType());

  /**
   * Fit the observations to a Gaussian mixture model (GMM) using the EM
   * algorithm.  The size of the vectors (indicating the number of components)
   * must already be set.
   *
   * @param observations List of observations to train on.
   * @param means Vector to store trained means in.
   * @param covariances Vector to store trained covariances in.
   * @param weights Vector to store a priori weights in.
   */
  void Estimate(const arma::mat& observations,
                std::vector<arma::vec>& means,
                std::vector<arma::mat>& covariances,
                arma::vec& weights);

  /**
   * Fit the observations to a Gaussian mixture model (GMM) using the EM
   * algorithm, taking into account the probabilities of each point being from
   * this mixture.  The size of the vectors (indicating the number of
   * components) must already be set.
   *
   * @param observations List of observations to train on.
   * @param probabilities Probability of each point being from this model.
   * @param means Vector to store trained means in.
   * @param covariances Vector to store trained covariances in.
   * @param weights Vector to store a priori weights in.
   */
  void Estimate(const arma::mat& observations,
                const arma::vec& probabilities,
                std::vector<arma::vec>& means,
                std::vector<arma::mat>& covariances,
                arma::vec& weights);

  //! Get the clusterer.
  const InitialClusteringType& Clusterer() const { return clusterer; }
  //! Modify the clusterer.
  InitialClusteringType& Clusterer() { return clusterer; }

  //! Get the maximum number of iterations of the EM algorithm.
  size_t MaxIterations() const { return maxIterations; }
  //! Modify the maximum number of iterations of the EM algorithm.
  size_t& MaxIterations() { return maxIterations; }

  //! Get the tolerance for the convergence of the EM algorithm.
  double Tolerance() const { return tolerance; }
  //! Modify the tolerance for the convergence of the EM algorithm.
  double& Tolerance() { return tolerance; }

  //! Get whether or not the covariance matrices are forced to be positive
  //! definite.
  bool ForcePositive() const { return forcePositive; }
  //! Modify whether or not the covariance matrices are forced to be positive
  //! definite.
  bool& ForcePositive() { return forcePositive; }

 private:
  /**
   * Run the clusterer, and then turn the cluster assignments into Gaussians.
   * This is a helper function for both overloads of Estimate().  The vectors
   * must be already set to the number of clusters.
   *
   * @param observations List of observations.
   * @param means Vector to store means in.
   * @param covariances Vector to store covariances in.
   * @param weights Vector to store a priori weights in.
   */
  void InitialClustering(const arma::mat& observations,
                         std::vector<arma::vec>& means,
                         std::vector<arma::mat>& covariances,
                         arma::vec& weights);

  /**
   * Calculate the log-likelihood of a model.  Yes, this is reimplemented in the
   * GMM code.  Intuition suggests that the log-likelihood is not the best way
   * to determine if the EM algorithm has converged.
   *
   * @param data Data matrix.
   * @param means Vector of means.
   * @param covariances Vector of covariance matrices.
   * @param weights Vector of a priori weights.
   */
  double LogLikelihood(const arma::mat& data,
                       const std::vector<arma::vec>& means,
                       const std::vector<arma::mat>& covariances,
                       const arma::vec& weights) const;

  //! Maximum iterations of EM algorithm.
  size_t maxIterations;
  //! Tolerance for convergence of EM.
  double tolerance;
  //! Whether or not to force positive definiteness of covariance matrices.
  bool forcePositive;
  //! Object which will perform the clustering.
  InitialClusteringType clusterer;
};

}; // namespace gmm
}; // namespace mlpack

// Include implementation.
#include "em_fit_impl.hpp"

#endif
