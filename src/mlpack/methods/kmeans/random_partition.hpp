/**
 * @file random_partition.hpp
 * @author Ryan Curtin
 *
 * Very simple partitioner which partitions the data randomly into the number of
 * desired clusters.  Used as the default InitialPartitionPolicy for KMeans.
 */
#ifndef __MLPACK_METHODS_KMEANS_RANDOM_PARTITION_HPP
#define __MLPACK_METHODS_KMEANS_RANDOM_PARTITION_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace kmeans {

/**
 * A very simple partitioner which partitions the data randomly into the number
 * of desired clusters.  It has no parameters, and so an instance of the class
 * is not even necessary.
 */
class RandomPartition
{
 public:
  //! Empty constructor, required by the InitialPartitionPolicy policy.
  RandomPartition() { }

  /**
   * Partition the given dataset into the given number of clusters.  Assignments
   * are random, and the number of points in each cluster should be equal (or
   * approximately equal).
   *
   * @tparam MatType Type of data (arma::mat or arma::spmat).
   * @param data Dataset to partition.
   * @param clusters Number of clusters to split dataset into.
   * @param assignments Vector to store cluster assignments into.  Values will
   *     be between 0 and (clusters - 1).
   */
  template<typename MatType>
  inline static void Cluster(const MatType& data,
                             const size_t clusters,
                             arma::Col<size_t>& assignments)
  {
    // Implementation is so simple we'll put it here in the header file.
    assignments = arma::shuffle(arma::linspace<arma::Col<size_t> >(0,
        (clusters - 1), data.n_cols));
  }
};

};
};

#endif
