/**
 * @file nmf_test.cpp
 * @author Mohan Rajendran
 *
 * Test file for NMF class.
 */
#include <mlpack/core.hpp>
#include <mlpack/methods/nmf/nmf.hpp>
#include <mlpack/methods/nmf/random_acol_init.hpp>
#include <mlpack/methods/nmf/mult_div_update_rules.hpp>
#include <mlpack/methods/nmf/als_update_rules.hpp>

#include <boost/test/unit_test.hpp>
#include "old_boost_test_definitions.hpp"

BOOST_AUTO_TEST_SUITE(NMFTest);

using namespace std;
using namespace arma;
using namespace mlpack;
using namespace mlpack::nmf;

/**
 * Check the if the product of the calculated factorization is close to the
 * input matrix. Default case
 */
BOOST_AUTO_TEST_CASE(NMFDefaultTest)
{
  mat w = randu<mat>(20, 16);
  mat h = randu<mat>(16, 20);
  mat v = w * h;
  size_t r = 16;

  NMF<> nmf;
  nmf.Apply(v, r, w, h);

  mat wh = w * h;

  for (size_t row = 0; row < 5; row++)
    for (size_t col = 0; col < 5; col++)
      BOOST_REQUIRE_CLOSE(v(row, col), wh(row, col), 10.0);
}

/**
 * Check the if the product of the calculated factorization is close to the
 * input matrix. Random Acol Initialization Distance Minimization Update
 */
BOOST_AUTO_TEST_CASE(NMFAcolDistTest)
{
  mat w = randu<mat>(20, 16);
  mat h = randu<mat>(16, 20);
  mat v = w * h;
  size_t r = 16;

  NMF<RandomAcolInitialization<> > nmf;
  nmf.Apply(v, r, w, h);

  mat wh = w * h;

  for (size_t row = 0; row < 5; row++)
    for (size_t col = 0; col < 5; col++)
      BOOST_REQUIRE_CLOSE(v(row, col), wh(row, col), 10.0);
}

/**
 * Check the if the product of the calculated factorization is close to the
 * input matrix. Random Initialization Divergence Minimization Update
 */
BOOST_AUTO_TEST_CASE(NMFRandomDivTest)
{
  mat w = randu<mat>(20, 16);
  mat h = randu<mat>(16, 20);
  mat v = w * h;
  size_t r = 16;

  NMF<RandomInitialization,
      WMultiplicativeDivergenceRule,
      HMultiplicativeDivergenceRule> nmf;
  nmf.Apply(v, r, w, h);

  mat wh = w * h;

  for (size_t row = 0; row < 5; row++)
    for (size_t col = 0; col < 5; col++)
      BOOST_REQUIRE_CLOSE(v(row, col), wh(row, col), 10.0);
}

/**
 * Check that the product of the calculated factorization is close to the
 * input matrix.  This uses the random initialization and alternating least
 * squares update rule.
 */
BOOST_AUTO_TEST_CASE(NMFALSTest)
{
  mat w = randu<mat>(20, 16);
  mat h = randu<mat>(16, 20);
  mat v = w * h;
  size_t r = 16;

  NMF<RandomInitialization,
      WAlternatingLeastSquaresRule,
      HAlternatingLeastSquaresRule> nmf;
  nmf.Apply(v, r, w, h);

  mat wh = w * h;

  for (size_t row = 0; row < 5; row++)
    for (size_t col = 0; col < 5; col++)
      BOOST_REQUIRE_CLOSE(v(row, col), wh(row, col), 12.0);
}

BOOST_AUTO_TEST_SUITE_END();
