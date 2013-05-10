/**
 * @file kernel_pca_impl.hpp
 * @author Ajinkya Kale
 * @author Marcus Edel
 *
 * Implementation of KernelPCA class to perform Kernel Principal Components
 * Analysis on the specified data set.
 */
#ifndef __MLPACK_METHODS_KERNEL_PCA_KERNEL_PCA_IMPL_HPP
#define __MLPACK_METHODS_KERNEL_PCA_KERNEL_PCA_IMPL_HPP

// In case it hasn't already been included.
#include "kernel_pca.hpp"

#include <iostream>

namespace mlpack {
namespace kpca {

template <typename KernelType>
arma::mat GetKernelMatrix(KernelType kernel, arma::mat transData);

template <typename KernelType>
KernelPCA<KernelType>::KernelPCA(const KernelType kernel,
                                 const bool centerTransformedData) :
      kernel(kernel),
      centerTransformedData(centerTransformedData)
{ }

//! Apply Kernel Principal Component Analysis to the provided data set.
template <typename KernelType>
void KernelPCA<KernelType>::Apply(const arma::mat& data,
                                  arma::mat& transformedData,
                                  arma::vec& eigval,
                                  arma::mat& eigvec)
{
  // Construct the kernel matrix.
  arma::mat kernelMatrix;
  GetKernelMatrix(data, kernelMatrix);

  // For PCA the data has to be centered, even if the data is centered.  But it
  // is not guaranteed that the data, when mapped to the kernel space, is also
  // centered. Since we actually never work in the feature space we cannot
  // center the data.  So, we perform a "psuedo-centering" using the kernel
  // matrix.

  // The matrix of ones, made below, may be somewhat large in memory.  The
  // centering expression might be optimizable to avoid creation of ones.
  arma::mat ones = arma::ones<arma::mat>(kernelMatrix.n_rows,
      kernelMatrix.n_cols);
  arma::mat centeredKernelMatrix = kernelMatrix - (ones * kernelMatrix) -
      (kernelMatrix * ones) + (ones * kernelMatrix * ones);

  // Eigendecompose the centered kernel matrix.
  arma::eig_sym(eigval, eigvec, centeredKernelMatrix);

  // Swap the eigenvalues since they are ordered backwards (we need largest to
  // smallest).
  for (size_t i = 0; i < floor(eigval.n_elem / 2.0); ++i)
    eigval.swap_rows(i, (eigval.n_elem - 1) - i);

  // Flip the coefficients to produce the same effect.
  eigvec = arma::fliplr(eigvec);

  transformedData = eigvec.t() * centeredKernelMatrix;

  // Center the transformed data, if the user asked for it.
  if (centerTransformedData)
  {
    arma::colvec transformedDataMean = arma::mean(transformedData, 1);
    transformedData = transformedData - (transformedDataMean *
        arma::ones<arma::rowvec>(transformedData.n_cols));
  }
}

//! Apply Kernel Principal Component Analysis to the provided data set.
template <typename KernelType>
void KernelPCA<KernelType>::Apply(const arma::mat& data,
                                  arma::mat& transformedData,
                                  arma::vec& eigVal)
{
  arma::mat coeffs;
  Apply(data, transformedData, eigVal, coeffs);
}

//! Use KPCA for dimensionality reduction.
template <typename KernelType>
void KernelPCA<KernelType>::Apply(arma::mat& data, const size_t newDimension)
{
  arma::mat coeffs;
  arma::vec eigVal;

  Apply(data, data, eigVal, coeffs);

  if (newDimension < coeffs.n_rows && newDimension > 0)
    data.shed_rows(newDimension, data.n_rows - 1);
}

//! Construct the kernel matrix.
template <typename KernelType>
void KernelPCA<KernelType>::GetKernelMatrix(const arma::mat& data,
                                            arma::mat& kernelMatrix)
{
  // Resize the kernel matrix to the right size.
  kernelMatrix.set_size(data.n_cols, data.n_cols);

  // Note that we only need to calculate the upper triangular part of the kernel
  // matrix, since it is symmetric.  This helps minimize the number of kernel
  // evaluations.
  for (size_t i = 0; i < data.n_cols; ++i)
  {
    for (size_t j = i; j < data.n_cols; ++j)
    {
      // Evaluate the kernel on these two points.
      kernelMatrix(i, j) = kernel.Evaluate(data.unsafe_col(i),
                                           data.unsafe_col(j));
    }
  }

  // Copy to the lower triangular part of the matrix.
  for (size_t i = 1; i < data.n_cols; ++i)
    for (size_t j = 0; j < i; ++j)
      kernelMatrix(i, j) = kernelMatrix(j, i);
}

}; // namespace mlpack
}; // namespace kpca

#endif
