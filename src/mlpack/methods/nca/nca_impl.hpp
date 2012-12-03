/**
 * @file nca_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of templated NCA class.
 */
#ifndef __MLPACK_METHODS_NCA_NCA_IMPL_HPP
#define __MLPACK_METHODS_NCA_NCA_IMPL_HPP

// In case it was not already included.
#include "nca.hpp"

namespace mlpack {
namespace nca {

// Just set the internal matrix reference.
template<typename MetricType, template<typename> class OptimizerType>
NCA<MetricType, OptimizerType>::NCA(const arma::mat& dataset,
                                    const arma::uvec& labels,
                                    MetricType metric) :
    dataset(dataset),
    labels(labels),
    metric(metric),
    errorFunction(dataset, labels, metric),
    optimizer(OptimizerType<SoftmaxErrorFunction<MetricType> >(errorFunction))
{ /* Nothing to do. */ }

template<typename MetricType, template<typename> class OptimizerType>
void NCA<MetricType, OptimizerType>::LearnDistance(arma::mat& outputMatrix)
{
  // See if we were passed an initialized matrix.
  if ((outputMatrix.n_rows != dataset.n_rows) ||
      (outputMatrix.n_cols != dataset.n_rows))
    outputMatrix.eye(dataset.n_rows, dataset.n_rows);

  Timer::Start("nca_sgd_optimization");

  optimizer.Optimize(outputMatrix);

  Timer::Stop("nca_sgd_optimization");
}

}; // namespace nca
}; // namespace mlpack

#endif
