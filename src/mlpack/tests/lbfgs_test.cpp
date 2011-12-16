/**
 * @file lbfgs_test.cpp
 *
 * Tests the L-BFGS optimizer on a couple test functions.
 *
 * @author Ryan Curtin (gth671b@mail.gatech.edu)
 */
#include <mlpack/core.hpp>
#include <mlpack/core/optimizers/lbfgs/lbfgs.hpp>
#include <mlpack/core/optimizers/lbfgs/test_functions.hpp>

#include <boost/test/unit_test.hpp>

using namespace mlpack::optimization;
using namespace mlpack::optimization::test;

BOOST_AUTO_TEST_SUITE(LBFGSTest);
#ifdef FOOBAR
/**
 * Tests the L-BFGS optimizer using the Rosenbrock Function.
 */
BOOST_AUTO_TEST_CASE(RosenbrockFunction)
{
  RosenbrockFunction f;
  L_BFGS<RosenbrockFunction> lbfgs(f, 10);

  arma::vec coords = f.GetInitialPoint();
  if (!lbfgs.Optimize(0, coords))
    BOOST_FAIL("L-BFGS optimization reported failure.");

  double finalValue = f.Evaluate(coords);

  BOOST_REQUIRE_SMALL(finalValue, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[0], 1, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[1], 1, 1e-5);
}

/**
 * Tests the L-BFGS optimizer using the Wood Function.
 */
BOOST_AUTO_TEST_CASE(WoodFunction)
{
  WoodFunction f;
  L_BFGS<WoodFunction> lbfgs(f, 10);

  arma::vec coords = f.GetInitialPoint();
  if (!lbfgs.Optimize(0, coords))
    BOOST_FAIL("L-BFGS optimization reported failure.");

  double finalValue = f.Evaluate(coords);

  BOOST_REQUIRE_SMALL(finalValue, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[0], 1, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[1], 1, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[2], 1, 1e-5);
  BOOST_REQUIRE_CLOSE(coords[3], 1, 1e-5);
}

/**
 * Tests the L-BFGS optimizer using the generalized Rosenbrock function.  This
 * is actually multiple tests, increasing the dimension by powers of 2, from 4
 * dimensions to 1024 dimensions.
 */
BOOST_AUTO_TEST_CASE(GeneralizedRosenbrockFunction)
{
  for (int i = 2; i < 10; i++)
  {
    // Dimension: powers of 2
    int dim = std::pow(2, i);

    GeneralizedRosenbrockFunction f(dim);
    L_BFGS<GeneralizedRosenbrockFunction> lbfgs(f, 20);

    arma::vec coords = f.GetInitialPoint();
    if (!lbfgs.Optimize(0, coords))
      BOOST_FAIL("L-BFGS optimization reported failure.");

    double finalValue = f.Evaluate(coords);

    // Test the output to make sure it is correct.
    BOOST_REQUIRE_SMALL(finalValue, 1e-5);
    for (int j = 0; j < dim; j++)
      BOOST_REQUIRE_CLOSE(coords[j], 1, 1e-5);
  }
}

/**
 * Tests the L-BFGS optimizer using the Rosenbrock-Wood combined function.  This
 * is a test on optimizing a matrix of coordinates.
 */
BOOST_AUTO_TEST_CASE(RosenbrockWoodFunction)
{
  RosenbrockWoodFunction f;
  L_BFGS<RosenbrockWoodFunction> lbfgs(f, 10);

  arma::mat coords = f.GetInitialPoint();
  if (!lbfgs.Optimize(0, coords))
    BOOST_FAIL("L-BFGS optimization reported failure.");

  double finalValue = f.Evaluate(coords);

  BOOST_REQUIRE_SMALL(finalValue, 1e-5);
  for (int row = 0; row < 4; row++)
  {
    BOOST_REQUIRE_CLOSE((coords(row, 0)), 1, 1e-5);
    BOOST_REQUIRE_CLOSE((coords(row, 1)), 1, 1e-5);
  }
}
#endif
BOOST_AUTO_TEST_SUITE_END();
