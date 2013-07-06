/**
 * @file metric_test.cpp
 *
 * Unit tests for the 'LMetric' class.
 */
#include <mlpack/core.hpp>
#include <mlpack/core/metrics/lmetric.hpp>
#include <boost/test/unit_test.hpp>
#include "old_boost_test_definitions.hpp"

using namespace std;
using namespace mlpack::metric;

BOOST_AUTO_TEST_SUITE(LMetricTest);

BOOST_AUTO_TEST_CASE(L1MetricTest)
{
  arma::vec a1(5);
  a1.randn();

  arma::vec b1(5);
  b1.randn();

  arma::Col<size_t> a2(5);
  a2 << 1 << 2 << 1 << 0 << 5;

  arma::Col<size_t> b2(5);
  b2 << 2 << 5 << 2 << 0 << 1;

  ManhattanDistance lMetric;

  BOOST_REQUIRE_CLOSE(arma::sum(arma::abs(a1 - b1)),
                      lMetric.Evaluate(a1, b1), 1e-5);

  BOOST_REQUIRE_CLOSE(arma::sum(arma::abs(a2 - b2)),
                      lMetric.Evaluate(a2, b2), 1e-5);
}

BOOST_AUTO_TEST_CASE(L2MetricTest)
{
  arma::vec a1(5);
  a1.randn();

  arma::vec b1(5);
  b1.randn();

  arma::Col<size_t> a2(5);
  a2 << 1 << 2 << 1 << 0 << 5;

  arma::Col<size_t> b2(5);
  b2 << 2 << 5 << 2 << 0 << 1;

  EuclideanDistance lMetric;

  BOOST_REQUIRE_CLOSE(sqrt(arma::sum(arma::square(a1 - b1))),
                      lMetric.Evaluate(a1, b1), 1e-5);

  BOOST_REQUIRE_CLOSE(sqrt(arma::sum(arma::square(a2 - b2))),
                      lMetric.Evaluate(a2, b2), 1e-5);
}

BOOST_AUTO_TEST_CASE(LINFMetricTest)
{
  arma::vec a1(5);
  a1.randn();

  arma::vec b1(5);
  b1.randn();

  arma::Col<size_t> a2(5);
  a2 << 1 << 2 << 1 << 0 << 5;

  arma::Col<size_t> b2(5);
  b2 << 2 << 5 << 2 << 0 << 1;

  EuclideanDistance lMetric;

  BOOST_REQUIRE_CLOSE(arma::max(arma::abs(a1 - b1)),
                      lMetric.Evaluate(a1, b1), 1e-5);

  BOOST_REQUIRE_CLOSE(arma::max(arma::abs(a2 - b2)),
                      lMetric.Evaluate(a2, b2), 1e-5);
}

BOOST_AUTO_TEST_SUITE_END();
