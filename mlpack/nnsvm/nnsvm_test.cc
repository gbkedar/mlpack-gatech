/**
 * @file nnsvm_test.cc
 *
 * Test file for NNSVM class
 */

#include <fastlib/fastlib.h>
#include <armadillo>
#include <iostream>
#include "nnsvm.h"

#define BOOST_TEST_MODULE NNSVM Test
#include <boost/test/unit_test.hpp>

using namespace mlpack;
using namespace mlpack::nnsvm;

/***
 * simple nonnegative SVM test with small, synthetic dataset
 */
BOOST_AUTO_TEST_CASE(linear_kernel_test_1)
{
  // initialize the matrix
  arma::mat data;

  data << -1.0 <<  1.0 << 1.0 << arma::endr
       << -2.0 <<  2.0 << 1.0 << arma::endr
       << -3.0 <<  3.0 << 1.0 << arma::endr
       << -4.0 <<  4.0 << 1.0 << arma::endr
       <<  1.0 << -1.0 << 0.0 << arma::endr
       <<  2.0 << -2.0 << 0.0 << arma::endr
       <<  3.0 << -3.0 << 0.0 << arma::endr
       <<  4.0 << -4.0 << 0.0 << arma::endr;

  // test the linear kernel
  NNSVM<SVMLinearKernel>* nnsvm = new NNSVM<SVMLinearKernel>();

  nnsvm->InitTrain(data, 2);
  double calculatedThreshold = nnsvm->getThreshold();
  size_t calculatedSupportVectorCount = nnsvm->getSupportVectorCount();
  const arma::vec calculatedSupportVectorCoefficients =
                    nnsvm->getSupportVectorCoefficients();
  const arma::vec calculatedWeightVector = nnsvm->getWeightVector();

  // check for correctness on the linear kernel
  BOOST_REQUIRE(calculatedSupportVectorCount == 3);
  BOOST_REQUIRE_CLOSE(calculatedThreshold, -1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[0],
                      3.7499785159728178, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[1],
                      6.2500214840271884, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[2], -10.000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[0], 0.00000000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[1], 0.00000000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[2], 0.00000000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[3], 0.00017187221748210524, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[4], 0.00000000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[5], 0.00000000, 1e-5);
  BOOST_REQUIRE_CLOSE(calculatedWeightVector[6], 0.00000000, 1e-5);

  // Clean the memory.
  delete nnsvm;
}
//BOOST_AUTO_TEST_CASE(linear_kernel_test_2)
//{
//  // initialize the matrix
//  arma::mat data;
//
//  data << 2.8532 << 0.6808 << -18 << 6.5 << -26.23 << -273.67 << 3.1747
//          << 1.4824 << 2.0161 << -11.142 << -31.166 << 0 << -1.0324
//          << -8.2685 << 3.48 << 8.545 << 6.575 << -7.89 << 1.9919 << -7
//          << 1 << arma::endr
//       << 0.0578 << 1.3971 << -24 << -12.1 << -100.13 << -553.18 << 2.3804
//          << -0.5607 << 4.5496 << -24.574 << -24.968 << -1 << -13.534 << -31.45
//          << 1.54 << -13.05 << -11.725 << -2.55 << 1.8087 << 10 << 1
//          << arma::endr
//       << 0.1804 << 1.6302 << -26 << -6.4 << -81.28 << -529.73 << 2.0123
//          << 1.0011 << 2.1322 << -38.049 << -22.548 << -1 << -17.921
//          << -29.786 << -4.2 << -5.885 << -12.065 << -0.18 << 3.7498
//          << 8 << 1 << arma::endr
//       << 2.2685 << 0.77 << -26 << -5 << -63.74 << -619.41 << 3.79 << 1.4824
//          << 2.6626 << -11.553 << -10.084 << -1 << -23.293 << -24.595 << 5.06
//          << 1.81 << -6.8 << -2.74 << 1.4367 << 2 << 1 << arma::endr
//       << 1.1094 << 1.3512 << -27 << -5.9 << -97.98 << -888.82 << 4.7719
//          << -0.8833 << 7.0048 << -41.996 << -25.473 << 0 << -17.197 << -31.864
//          << 2.96 << -12.92 << -21.625 << -4.66 << 4.1161 << 14 << 1
//          << arma::endr
//       << -0.7448 << 0.639 << -33 << -4.3 << -121.87 << -1240.2 << 2.574
//          << -11.96 << -1.4605 << -23.163 << -41.738 << 0 << -26.078
//          << -65.459 << -3.46 << -43.665 << -47.125 << 0.36 << 1.5793
//          << 11 << 1 << arma::endr
//       << 2.1242 << 1.0001 << -27 << -1.5 << -85.01 << -753.81 << 5.8026
//          << 0.4733 << 7.3244 << -28.602 << -22.096 << -1 << -25.72 << -5.058
//          << 1.96 << -2.715 << -36.435 << 6.23 << 4.0664 << 4 << 1 << arma::endr
//       << 1.5147 << 1.5999 << -30 << 9.5 << -48.64 << -552.72 << 6.8646
//          << -2.7514 << 9.765 << -32.567 << -26.009 << -1 << -21.244 << -42.178
//          << -4.38 << -14.655 << -35.8 << -8.5 << 3.5578 << -3 << 1
//          << arma::endr
//       << 0.2762 << 0.4498 << -33 << -14.5 << -108.06 << -1848.7 << 3.3279
//          << -4.7404 << 8.3787 << -69.738 << -45.989 << -2 << -20.988 << -87.26
//          << 5.26 << -41.045 << -46.53 << 2.92 << 0.2006 << 17 << 1
//          << arma::endr
//       << -0.8361 << 0.5906 << -35 << -9.9 << -123.2 << -1535.6 << 1.7498
//          << -9.412 << 4.8324 << -61.991 << -45.081 << -1 << -28.325 << -85.874
//          << -10.38 << -36.53 << -51.315 << -4.12 << 1.5208 << 28 << 1
//          << arma::endr
//       << 3.4862 << 0.1746 << -8 << -12.6 << -48.05 << -74.518 << -5.3072 << 0
//          << -0.6322 << 14.706 << 4.6319 << -1 << -5.1945 << -10.268 << -13.8
//          << -18.11 << -2.625 << -9.7 << 0.0399 << -11 << 0 << arma::endr
//       << -1.332 << -0.5399 << 6 << -11.5 << -7.1 << -41.538 << -0.4827
//          << 0.1494 << 2.6331 << 13.711 << -20.669 << 1 << -2.307 << -16.457
//          << -5.3 << -21.65 << 3.57 << 6.82 << -1.4101 << -6 << 0
//          << arma::endr
//       << 0.6689 << 0.1837 << 2 << -1.9 << 10.22 << 176.41 << -1.2294 << 0
//          << -1.7885 << -16.553 << -2.9748 << 0 << 9.304 << 10.097 << -4.32
//          << -7.325 << -6.375 << 13.96 << -0.1116 << 25 << 0 << arma::endr
//       << -0.9639 << -0.6035 << 15 << 17.7 << 82.64 << 849.54 << 1.7272
//          << 2.3738 << -2.3887 << -4.4992 << 31.715 << -1 << 18.632 << -4.4991
//          << 13 << 8.955 << 12.725 << -19.04 << 0.218 << -24 << 0 << arma::endr
//       << -3.5669 << -0.4114 << 19 << 11.8 << 84.35 << 749.12 << -3.8668
//          << 1.9631 << 3.2414 << 25.695 << 28.835 << 1 << 30.208 << 34.002
//          << 14.78 << 46.875 << 24.84 << 8.1 << -1.7294 << -7 << 0 << arma::endr
//       << 1.5881 << -0.023 << 24 << 6.8 << 108.42 << 1255.7 << 2.0539 << 12.079
//          << 1.7954 << 21.008 << 33.312 << 0 << 27.819 << 22.083 << 15.94
//          << 71.565 << 49.805 << 3.94 << 0.3416 << -12 << 0 << arma::endr
//       << -1.5349 << -0.5047 << 10 << -12.2 << 12.26 << 476.02 << -4.4551 << 0
//          << -7.213 << 7.56 << 2.4955 << 0 << 18.81 << -13.185 << -11 << 9.86
//          << -12.555 << -3.63 << -2.4305 << -7 << 0 << arma::endr
//       << 1.9474 << -1.9588 << 26 << 29.8 << 156.44 << 1885.6 << 1.7485
//          << 12.592 << -4.1963 << 17.719 << 45.104 << 0 << 29.739 << 82.289
//          << 17 << 62.15 << 41.805 << -13.57 << 1.4464 << -48 << 0
//          << arma::endr
//       << 1.9065 << -0.1575 << 24 << -8.5 << 59.85 << 577 << -6.8442 << -0.4728
//          << -1.5632 << 19.76 << 13.641 << -2 << 32.432 << 34.894 << -0.9
//          << 52.775 << 23.125 << -24 << -8.2289 << -18 << 0 << arma::endr
//       << -0.6037 << -0.4235 << 21 << 18.6 << 137.71 << 1506.4 << -0.9627
//          << 11.03 << -2.7523 << 47.119 << 58.999 << 1 << 21.233 << 74.764
//          << 21.96 << 72.49 << 54.675 << -10.04 << 1.916 << -12 << 0
//          << arma::endr;
//
//  // test the linear kernel
//  NNSVM<SVMLinearKernel>* nnsvm = new NNSVM<SVMLinearKernel>();
//
//  nnsvm->InitTrain(data, 2);
//  double calculatedThreshold = nnsvm->getThreshold();
//  size_t calculatedSupportVectorCount = nnsvm->getSupportVectorCount();
//  const arma::vec calculatedSupportVectorCoefficients =
//                    nnsvm->getSupportVectorCoefficients();
//  const arma::vec calculatedWeightVector = nnsvm->getWeightVector();
//
//  // check for correctness on the linear kernel
//  BOOST_REQUIRE(calculatedSupportVectorCount == 6);
//  BOOST_REQUIRE_CLOSE(calculatedThreshold, -39.793784137999701, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[0],
//                      0.06875628588658407, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[1],
//                      3.1607079296638054, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[2],
//                      0.0039166013511236601, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[3],
//                      3.7130510722124139, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[4],
//                      0.43343566949350276, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedSupportVectorCoefficients[5],
//                      -10.000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[0], 0.00000000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[1], 0.00000000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[2], 0.00000000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[3], 0.00000000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[4], 0.053222444790909262, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[5], 0.00000000, 1e-5);
//  BOOST_REQUIRE_CLOSE(calculatedWeightVector[6], 0.00000000, 1e-5);
//
//  // Clean the memory.
//  delete nnsvm;
//}
///***
// * Test the dual-tree nearest-neighbors method with the naive method.  This
// * uses both a query and reference dataset.
// *
// * Errors are produced if the results are not identical.
// */
//BOOST_AUTO_TEST_CASE(dual_tree_vs_naive_1)
//{
//  arma::mat data_for_tree_;
//
//  // Hard-coded filename: bad!
//  if (data::Load("test_data_3_1000.csv", data_for_tree_) != true)
//  {
//    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");
//  }
//
//  // Set up matrices to work with.
//  arma::mat dual_query(data_for_tree_);
//  arma::mat dual_references(data_for_tree_);
//  arma::mat naive_query(data_for_tree_);
//  arma::mat naive_references(data_for_tree_);
//
//  AllkNN allknn_(dual_query, dual_references, 20, 5);
//  AllkNN naive_(naive_query, naive_references, 1 /* leaf_size ignored */, 5,
//      AllkNN::NAIVE);
//
//  arma::Col<size_t> resulting_neighbors_tree;
//  arma::vec distances_tree;
//  allknn_.ComputeNeighbors(resulting_neighbors_tree, distances_tree);
//
//  arma::Col<size_t> resulting_neighbors_naive;
//  arma::vec distances_naive;
//  naive_.ComputeNeighbors(resulting_neighbors_naive, distances_naive);
//
//  for (size_t i = 0; i < resulting_neighbors_tree.n_elem; i++)
//  {
//    BOOST_REQUIRE(resulting_neighbors_tree[i] == resulting_neighbors_naive[i]);
//    BOOST_REQUIRE_CLOSE(distances_tree[i], distances_naive[i], 1e-5);
//  }
//}
//
///***
// * Test the dual-tree nearest-neighbors method with the naive method.  This uses
// * only a reference dataset.
// *
// * Errors are produced if the results are not identical.
// */
//BOOST_AUTO_TEST_CASE(dual_tree_vs_naive_2)
//{
//  arma::mat data_for_tree_;
//
//  // Hard-coded filename: bad!
//  // Code duplication: also bad!
//  if (data::Load("test_data_3_1000.csv", data_for_tree_) != true)
//  {
//    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");
//  }
//
//  // Set up matrices to work with (may not be necessary with no ALIAS_MATRIX?).
//  arma::mat dual_query(data_for_tree_);
//  arma::mat naive_query(data_for_tree_);
//
//  AllkNN allknn_(dual_query, 20, 1);
//  AllkNN naive_(naive_query, 1 /* leaf_size ignored with naive */, 1,
//      AllkNN::NAIVE);
//
//  arma::Col<size_t> resulting_neighbors_tree;
//  arma::vec distances_tree;
//  allknn_.ComputeNeighbors(resulting_neighbors_tree, distances_tree);
//
//  arma::Col<size_t> resulting_neighbors_naive;
//  arma::vec distances_naive;
//  naive_.ComputeNeighbors(resulting_neighbors_naive, distances_naive);
//
//  for (size_t i = 0; i < resulting_neighbors_tree.n_elem; i++) {
//    BOOST_REQUIRE(resulting_neighbors_tree[i] == resulting_neighbors_naive[i]);
//    BOOST_REQUIRE_CLOSE(distances_tree[i], distances_naive[i], 1e-5);
//  }
//}
//
///***
// * Test the single-tree nearest-neighbors method with the naive method.  This
// * uses only a reference dataset.
// *
// * Errors are produced if the results are not identical.
// */
//BOOST_AUTO_TEST_CASE(single_tree_vs_naive)
//{
//  arma::mat data_for_tree_;
//
//  // Hard-coded filename: bad!
//  // Code duplication: also bad!
//  if (data::Load("test_data_3_1000.csv", data_for_tree_) != true)
//    BOOST_FAIL("Cannot load test dataset test_data_3_1000.csv!");
//
//  // Set up matrices to work with (may not be necessary with no ALIAS_MATRIX?).
//  arma::mat single_query(data_for_tree_);
//  arma::mat naive_query(data_for_tree_);
//
//  AllkNN allknn_(single_query, 20, 5, AllkNN::MODE_SINGLE);
//  AllkNN naive_(naive_query, 1 /* leaf_size ignored with naive */, 5,
//      AllkNN::NAIVE);
//
//  arma::Col<size_t> resulting_neighbors_tree;
//  arma::vec distances_tree;
//  allknn_.ComputeNeighbors(resulting_neighbors_tree, distances_tree);
//
//  arma::Col<size_t> resulting_neighbors_naive;
//  arma::vec distances_naive;
//  naive_.ComputeNeighbors(resulting_neighbors_naive, distances_naive);
//
//  for (size_t i = 0; i < resulting_neighbors_tree.n_elem; i++) {
//    BOOST_REQUIRE(resulting_neighbors_tree[i] == resulting_neighbors_naive[i]);
//    BOOST_REQUIRE_CLOSE(distances_tree[i], distances_naive[i], 1e-5);
//  }
//}
