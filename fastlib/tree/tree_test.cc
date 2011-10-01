/**
 * @file uselapack_test.cc
 *
 * Tests for LAPACK integration.
 */

#include "bounds.h"
#include "spacetree.h"
#include "../../mlpack/core/kernels/lmetric.h"

#define BOOST_TEST_MODULE Tree_Test
#include <boost/test/unit_test.hpp>

using namespace mlpack;
using namespace mlpack::tree;
using namespace mlpack::kernel;
using namespace mlpack::bound;

BOOST_AUTO_TEST_CASE(TestDHrectPeriodicBound) {

  arma::vec box(2);
  box[0] = 5.5;
  box[1] = 2.5;
  DHrectPeriodicBound<2> p2(box);
  DHrectPeriodicBound<2> p1(box);

  //Two squares with length 2, with 1 distance apart along the x-axis.
  p1[0] = Range(0.0, 2.0);
  p1[1] = Range(0.0, 2.0);
  p2[0] = Range(3.0, 5.0);
  p2[1] = Range(0.0, 2.0);

  //A point at (1,1)
  arma::vec vector(2);
  vector[0] = 1.0;
  vector[1] = 1.0;

  BOOST_REQUIRE(p1.Contains(vector));
  BOOST_REQUIRE(!p2.Contains(vector));
  BOOST_REQUIRE_CLOSE(p1.CalculateMaxDistanceSq(), 8.0, 1e-5);

  p2.CalculateMidpoint(vector);
  BOOST_REQUIRE_CLOSE(vector[0], 4.0, 1e-5);
  BOOST_REQUIRE_CLOSE(vector[1], 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(p1.MinDistanceSq(p2), 0.25, 1e-5);

  vector[0] = 2.0;
  vector[1] = 2.5;
//  Do not work properly yet.
//  BOOST_REQUIRE_CLOSE(p1.MinDistanceSq(vector), 5.0, 1e-5);
//  BOOST_REQUIRE_CLOSE(p1.MaxDistanceSq(vector), 20.0, 1e-5);
//  BOOST_REQUIRE_CLOSE(p1.MaxDistanceSq(p2), 29.0, 1e-5);
  BOOST_REQUIRE_CLOSE(p1.MinDelta(p2, 3.0, 0), -1.5, 1e-5);
  BOOST_REQUIRE_CLOSE(p1.MaxDelta(p2, 3.0, 0), 1.5, 1e-5);

  Range range = p1.RangeDistanceSq(p2);
  BOOST_REQUIRE_CLOSE(range.lo, 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(range.hi, 29.0, 1e-5);

  range = p1.RangeDistanceSq(vector);
  BOOST_REQUIRE_CLOSE(range.lo, 0.25, 1e-5);
  BOOST_REQUIRE_CLOSE(range.hi, 10.25, 1e-5);

  BOOST_REQUIRE_CLOSE(p1.MinToMidSq(p2), 4.0, 1e-5);
  BOOST_REQUIRE_CLOSE(p1.MinimaxDistanceSq(p2), 9.0, 1e-5);
  BOOST_REQUIRE_CLOSE(p1.MidDistanceSq(p2), 9.0, 1e-5);

  vector[0] = 6.0;
  vector[1] = 3.0;
  p2 |= vector;
  p1 |= p2;
  BOOST_REQUIRE(p2.Contains(vector));

  vector[0] = 5.0;
  vector[1] = 2.0;
  BOOST_REQUIRE(p1.Contains(vector));

  p1[0] = Range(0.0, 2.0);
  p1[1] = Range(0.0, 2.0);
  arma::vec size(2);
  size[0] = 3.5;
  size[1] = 2.0;
  vector[0] = 3.0;
  vector[1] = 1.0;

  //Does not work yet.
//  DHrectPeriodicBound<> p3 = p1.Add(vector, size);
//  BOOST_REQUIRE(r3.Contains(vector));

//  DHrectPeriodicBound<> p3 = p1.Add(p2, size);
//  BOOST_REQUIRE(r3.Contains(vector));
}

/***
 * Ensure that a bound, by default, is empty and has no dimensionality.
 */
BOOST_AUTO_TEST_CASE(HRectBoundEmptyConstructor) {
  HRectBound<2> b;

  BOOST_REQUIRE_EQUAL(b.dim(), 0);
}

/***
 * Ensure that when we specify the dimensionality in the constructor, it is
 * correct, and the bounds are all the empty set.
 */
BOOST_AUTO_TEST_CASE(HRectBoundDimConstructor) {
  HRectBound<2> b(2); // We'll do this with 2 and 5 dimensions.

  BOOST_REQUIRE_EQUAL(b.dim(), 2);
  BOOST_REQUIRE_SMALL(b[0].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[1].width(), 1e-5);

  b = HRectBound<2>(5);

  BOOST_REQUIRE_EQUAL(b.dim(), 5);
  BOOST_REQUIRE_SMALL(b[0].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[1].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[2].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[3].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[4].width(), 1e-5);
}

/***
 * Test that clearing the dimensions resets the bound to empty.
 */
BOOST_AUTO_TEST_CASE(HRectBoundClear) {
  HRectBound<2> b(2); // We'll do this with two dimensions only.

  b[0] = Range(0.0, 2.0);
  b[1] = Range(2.0, 4.0);

  // Now we just need to make sure that we clear the range.
  b.Clear();

  BOOST_REQUIRE_SMALL(b[0].width(), 1e-5);
  BOOST_REQUIRE_SMALL(b[1].width(), 1e-5);
}

/***
 * Ensure that we get the correct centroid for our bound.
 */
BOOST_AUTO_TEST_CASE(HRectBoundCentroid) {
  // Create a simple 3-dimensional bound.
  HRectBound<2> b(3);

  b[0] = Range(0.0, 5.0);
  b[1] = Range(-2.0, -1.0);
  b[2] = Range(-10.0, 50.0);

  arma::vec centroid;

  b.Centroid(centroid);

  BOOST_REQUIRE_EQUAL(centroid.n_elem, 3);
  BOOST_REQUIRE_CLOSE(centroid[0], 2.5, 1e-5);
  BOOST_REQUIRE_CLOSE(centroid[1], -1.5, 1e-5);
  BOOST_REQUIRE_CLOSE(centroid[2], 20.0, 1e-5);
}

/***
 * Ensure that we calculate the correct minimum distance between a point and a
 * bound.
 */
BOOST_AUTO_TEST_CASE(HRectBoundMinDistancePoint) {
  // We'll do the calculation in three dimensions, and we'll use three cases for
  // the point: point is outside the bound; point is on the edge of the bound;
  // point is inside the bound.  In the latter two cases, the distance should be
  // zero.
  HRectBound<2> b(5);

  b[0] = Range(0.0, 2.0);
  b[1] = Range(1.0, 5.0);
  b[2] = Range(-2.0, 2.0);
  b[3] = Range(-5.0, -2.0);
  b[4] = Range(1.0, 2.0);

  arma::vec point = "-2.0 0.0 10.0 3.0 3.0";

  // This will be the Euclidean squared distance.
  BOOST_REQUIRE_CLOSE(b.MinDistance(point), 95.0, 1e-5);

  point = "2.0 5.0 2.0 -5.0 1.0";

  BOOST_REQUIRE_SMALL(b.MinDistance(point), 1e-5);

  point = "1.0 2.0 0.0 -2.0 1.5";

  BOOST_REQUIRE_SMALL(b.MinDistance(point), 1e-5);
}

BOOST_AUTO_TEST_CASE(TestHRectBound) {
  HRectBound<> r1(2);
  HRectBound<> r2(2);

  // Two squares with length 2, with 1 distance apart along the x-axis.
  r1[0] = Range(0.0, 2.0);
  r1[1] = Range(0.0, 2.0);
  r2[0] = Range(3.0, 5.0);
  r2[1] = Range(0.0, 2.0);

  // A point at (1,1)
  arma::vec vector(2);
  vector[0] = 1.0;
  vector[1] = 1.0;

  BOOST_REQUIRE(r1.Contains(vector));
  BOOST_REQUIRE(!r2.Contains(vector));

  r2.Centroid(vector);
  BOOST_REQUIRE_CLOSE(vector[0], 4.0, 1e-5);
  BOOST_REQUIRE_CLOSE(vector[1], 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(r1.MinDistance(r2), 1.0, 1e-5);

  vector[0] = 4.0;
  vector[1] = 2.0;
  BOOST_REQUIRE_CLOSE(r1.MinDistance(r2, vector), 5.0, 1e-5);
  BOOST_REQUIRE_CLOSE(r1.MinDistance(vector), 4.0, 1e-5);
  BOOST_REQUIRE_CLOSE(r1.MaxDistance(vector), 20.0, 1e-5);
  BOOST_REQUIRE_CLOSE(r1.MaxDistance(r2), 29.0, 1e-5);

  Range range = r1.RangeDistance(r2);
  BOOST_REQUIRE_CLOSE(range.lo, 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(range.hi, 29.0, 1e-5);

  range = r1.RangeDistance(vector);
  BOOST_REQUIRE_CLOSE(range.lo, 4.0, 1e-5);
  BOOST_REQUIRE_CLOSE(range.hi, 20.0, 1e-5);

  BOOST_REQUIRE_CLOSE(r1.MinimaxDistance(r2), 9.0, 1e-5);
  BOOST_REQUIRE_CLOSE(r1.MidDistance(r2), 9.0, 1e-5);

  vector[0] = 6.0;
  vector[1] = 3.0;
  r2 |= vector;
  r1 |= r2;
  BOOST_REQUIRE(r2.Contains(vector));

  vector[0] = 5.0;
  vector[1] = 2.0;
  BOOST_REQUIRE(r1.Contains(vector));

  r1[0] = Range(0.0, 2.0);
  r1[1] = Range(0.0, 2.0);
  arma::vec size(2);
  size[0] = 3.5;
  size[1] = 2.0;
  vector[0] = 3.0;
  vector[1] = 1.0;

}

BOOST_AUTO_TEST_CASE(TestBallBound) {
  DBallBound<> b1;
  DBallBound<> b2;

  // Create two balls with a center distance of 1 from each other.
  // Give the first one a radius of 0.3 and the second a radius of 0.4.

  b1.center().set_size(3);
  b1.center()[0] = 1;
  b1.center()[1] = 2;
  b1.center()[2] = 3;
  b1.set_radius(0.3);

  b2.center().set_size(3);
  b2.center()[0] = 1;
  b2.center()[1] = 2;
  b2.center()[2] = 4;
  b2.set_radius(0.4);

  BOOST_REQUIRE_CLOSE(sqrt(b1.MinDistanceSq(b2)), 1-0.3-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.RangeDistanceSq(b2).hi), 1+0.3+0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.RangeDistanceSq(b2).lo), 1-0.3-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(b1.RangeDistance(b2).hi, 1+0.3+0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(b1.RangeDistance(b2).lo, 1-0.3-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.MinToMidSq(b2)), 1-0.3, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.MinimaxDistanceSq(b2)), 1-0.3+0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.MidDistanceSq(b2)), 1.0, 1e-5);

  BOOST_REQUIRE_CLOSE(sqrt(b2.MinDistanceSq(b1)), 1-0.3-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MaxDistanceSq(b1)), 1+0.3+0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.RangeDistanceSq(b1).hi), 1+0.3+0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.RangeDistanceSq(b1).lo), 1-0.3-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MinToMidSq(b1)), 1-0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MinimaxDistanceSq(b1)), 1-0.4+0.3, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MidDistanceSq(b1)), 1.0, 1e-5);

  BOOST_REQUIRE(b1.Contains(b1.center()));
  BOOST_REQUIRE(!b1.Contains(b2.center()));

  BOOST_REQUIRE(!b2.Contains(b1.center()));
  BOOST_REQUIRE(b2.Contains(b2.center()));
  arma::vec b2point(3); // a point that's within the radius bot not the center
  b2point[0] = 1.1;
  b2point[1] = 2.1;
  b2point[2] = 4.1;

  BOOST_REQUIRE(b2.Contains(b2point));

  BOOST_REQUIRE_SMALL(sqrt(b1.MinDistanceSq(b1.center())), 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.MinDistanceSq(b2.center())), 1 - 0.3, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MinDistanceSq(b1.center())), 1 - 0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b2.MaxDistanceSq(b1.center())), 1 + 0.4, 1e-5);
  BOOST_REQUIRE_CLOSE(sqrt(b1.MaxDistanceSq(b2.center())), 1 + 0.3, 1e-5);
}

/***
 * It seems as though Bill has stumbled across a bug where
 * BinarySpaceTree<>::count() returns something different than
 * BinarySpaceTree<>::count_.  So, let's build a simple tree and make sure they
 * are the same.
 */
BOOST_AUTO_TEST_CASE(tree_count_mismatch) {
  arma::mat dataset = "2.0 5.0 9.0 4.0 8.0 7.0;"
                      "3.0 4.0 6.0 7.0 1.0 2.0 ";

  // Leaf size of 1.
  IO::GetParam<int>("tree/leaf_size") = 1;
  BinarySpaceTree<HRectBound<2> > root_node(dataset);

  BOOST_REQUIRE(root_node.count() == 6);
  BOOST_REQUIRE(root_node.left()->count() == 3);
  BOOST_REQUIRE(root_node.left()->left()->count() == 2);
  BOOST_REQUIRE(root_node.left()->left()->left()->count() == 1);
  BOOST_REQUIRE(root_node.left()->left()->right()->count() == 1);
  BOOST_REQUIRE(root_node.left()->right()->count() == 1);
  BOOST_REQUIRE(root_node.right()->count() == 3);
  BOOST_REQUIRE(root_node.right()->left()->count() == 2);
  BOOST_REQUIRE(root_node.right()->left()->left()->count() == 1);
  BOOST_REQUIRE(root_node.right()->left()->right()->count() == 1);
  BOOST_REQUIRE(root_node.right()->right()->count() == 1);
}


BOOST_AUTO_TEST_CASE(kd_tree_test) {
  size_t max_points = 10000;
  size_t dimensions = 3;
  // Generate the dataset.
  srand(time(NULL));
  size_t size = rand() % max_points;
  arma::mat dataset = arma::mat(dimensions, size);
  arma::mat datacopy;

  // Mappings for post-sort verification of data.
  std::vector<size_t> new_to_old;
  std::vector<size_t> old_to_new;

  // Generate data.
  dataset.randu(dimensions, size);
  datacopy = dataset;

  // Check validity of tree data
  BinarySpaceTree<HRectBound<2> > root(dataset, new_to_old, old_to_new);

  BOOST_REQUIRE_EQUAL(root.count(), size);

  for(size_t i = 0; i < size; i++) {
    for(size_t j = 0; j < dimensions; j++) {
      // Check mappings.
      BOOST_REQUIRE_EQUAL(dataset(j, i), datacopy(j, new_to_old[i]));
      BOOST_REQUIRE_EQUAL(dataset(j, old_to_new[i]), datacopy(j, i));
    }
  }
}
