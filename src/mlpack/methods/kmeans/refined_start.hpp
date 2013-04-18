/**
 * @file refined_start.hpp
 * @author Ryan Curtin
 *
 * An implementation of Bradley and Fayyad's "Refining Initial Points for
 * K-Means clustering".  This class is meant to provide better initial points
 * for the k-means algorithm.
 */
#ifndef __MLPACK_METHODS_KMEANS_REFINED_START_HPP
#define __MLPACK_METHODS_KMEANS_REFINED_START_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace kmeans {

/**
 * A refined approach for choosing initial points for k-means clustering.  This
 * approach runs k-means several times on random subsets of the data, and then
 * clusters those solutions to select refined initial cluster assignments.  It
 * is an implementation of the following paper:
 *
 * @inproceedings{bradley1998refining,
 *   title={Refining initial points for k-means clustering},
 *   author={Bradley, Paul S and Fayyad, Usama M},
 *   booktitle={Proceedings of the Fifteenth International Conference on Machine
 *       Learning (ICML 1998)},
 *   volume={66},
 *   year={1998}
 * }
 */
class RefinedStart
{
 public:
  /**
   * Create the RefinedStart object, optionally specifying parameters for the
   * number of samplings to perform and the percentage of the dataset to use in
   * each sampling.
   */
  RefinedStart(const size_t samplings = 100,
               const double percentage = 0.02) :
      samplings(samplings), percentage(percentage) { }

  /**
   * Partition the given dataset into the given number of clusters according to
   * the random sampling scheme outlined in Bradley and Fayyad's paper.
   *
   * @tparam MatType Type of data (arma::mat or arma::sp_mat).
   * @param data Dataset to partition.
   * @param clusters Number of clusters to split dataset into.
   * @param assignments Vector to store cluster assignments into.  Values will
   *     be between 0 and (clusters - 1).
   */
  template<typename MatType>
  void Cluster(const MatType& data,
               const size_t clusters,
               arma::Col<size_t>& assignments) const;

  //! Get the number of samplings that will be performed.
  size_t Samplings() const { return samplings; }
  //! Modify the number of samplings that will be performed.
  size_t& Samplings() { return samplings; }

  //! Get the percentage of the data used by each subsampling.
  double Percentage() const { return percentage; }
  //! Modify the percentage of the data used by each subsampling.
  double& Percentage() { return percentage; }

 private:
  //! The number of samplings to perform.
  size_t samplings;
  //! The percentage of the data to use for each subsampling.
  double percentage;
};

}; // namespace kmeans
}; // namespace mlpack

// Include implementation.
#include "refined_start_impl.hpp"

#endif
