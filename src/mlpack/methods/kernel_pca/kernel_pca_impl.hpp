/**
 * @file kernelpca_impl.hpp
 * @author Ajinkya Kale
 *
 * Implementation of KernelPCA class to perform Kernel Principal Components
 * Analysis on the specified data set.
 */
#ifndef __MLPACK_METHODS_KERNEL_PCA_KERNEL_PCA_IMPL_HPP
#define __MLPACK_METHODS_KERNEL_PCA_KERNEL_PCA_IMPL_HPP

// In case it hasn't already been included.
#include "kernel_pca.hpp"

#include <iostream>

using namespace std; // This'll have to go before the release.

namespace mlpack {
namespace kpca {

template <typename KernelType>
KernelPCA<KernelType>::KernelPCA(const KernelType kernel,
                       const bool centerData,
                       const bool scaleData) :
      kernel(kernel),
      centerData(centerData),
      scaleData(scaleData)
{
}

/**
 * Apply Kernel Principal Component Analysis to the provided data set.
 *
 * @param data - Data matrix
 * @param transformedData - Data with KernelPCA applied
 * @param eigVal - contains eigen values in a column vector
 * @param coeff - KernelPCA Loadings/Coeffs/EigenVectors
 */
template <typename KernelType>
void KernelPCA<KernelType>::Apply(const arma::mat& data,
                                  arma::mat& transformedData,
                                  arma::vec& eigVal,
                                  arma::mat& coeffs)
{
  arma::mat transData = trans(data);

  if(centerData)
  {
    arma::rowvec means = arma::mean(transData, 0);
    transData = transData - arma::ones<arma::colvec>(transData.n_rows) * means;
  }
  if (scaleData)
  {
    transData = transData / (arma::ones<arma::colvec>(transData.n_rows) *
    stddev(transData, 0, 0));
  }
  arma::mat centeredData = trans(transData);
  arma::mat kernelMat = GetKernelMatrix(kernel, centeredData);
  arma::eig_sym(eigVal, coeffs, kernelMat);

  int n_eigVal = eigVal.n_elem;
  for(int i = 0; i < floor(n_eigVal / 2); i++)
    eigVal.swap_rows(i, (n_eigVal - 1) - i);

  coeffs = arma::fliplr(coeffs);

  transformedData = trans(coeffs) * data;
  arma::colvec transformedDataMean = arma::mean(transformedData, 1);
  transformedData = transformedData - (transformedDataMean * arma::ones<arma::rowvec>(transformedData.n_cols));
}

/**
 * Apply Kernel Principal Component Analysis to the provided data set.
 *
 * @param data - Data matrix
 * @param transformedData - Data with KernelPCA applied
 * @param eigVal - contains eigen values in a column vector
 */
template <typename KernelType>
void KernelPCA<KernelType>::Apply(const arma::mat& data,
                                  arma::mat& transformedData,
                                  arma::vec& eigVal)
{
  arma::mat coeffs;
  Apply(data, transformedData, eigVal, coeffs);
}

/**
 * Apply Dimensionality Reduction using Kernel Principal Component Analysis
 * to the provided data set.
 *
 * @param data - M x N Data matrix
 * @param newDimension - matrix consisting of N column vectors,
 * where each vector is the projection of the corresponding data vector
 * from data matrix onto the basis vectors contained in the columns of
 * coeff/eigen vector matrix with only newDimension number of columns chosen.
 */
template <typename KernelType>
void KernelPCA<KernelType>::Apply(arma::mat& data, const size_t newDimension)
{
  arma::mat coeffs;
  arma::vec eigVal;

  Apply(data, data, eigVal, coeffs);

  if (newDimension < coeffs.n_rows && newDimension > 0)
    data.shed_rows(newDimension, data.n_rows - 1);
}

}; // namespace mlpack
}; // namespace kpca

template <typename KernelType>
arma::mat GetKernelMatrix(KernelType kernel, arma::mat transData)
{
  arma::mat kernelMat(transData.n_rows, transData.n_rows);

  for(int i = 0; i < transData.n_rows; i++)
  {
    for(int j = 0; j < transData.n_rows; j++)
    {
      arma::vec v1 = trans(transData.row(i));
      arma::vec v2 = trans(transData.row(j));
      kernelMat(i, j) = kernel.Evaluate(v1, v2);
    }
  }

  return kernelMat;
}

#endif
