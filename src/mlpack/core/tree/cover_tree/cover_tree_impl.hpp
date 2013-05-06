/**
 * @file cover_tree_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of CoverTree class.
 */
#ifndef __MLPACK_CORE_TREE_COVER_TREE_COVER_TREE_IMPL_HPP
#define __MLPACK_CORE_TREE_COVER_TREE_COVER_TREE_IMPL_HPP

// In case it hasn't already been included.
#include "cover_tree.hpp"

#include <mlpack/core/util/string_util.hpp>
#include <string>

namespace mlpack {
namespace tree {

// Create the cover tree.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::CoverTree(
    const arma::mat& dataset,
    const double base,
    MetricType* metric) :
    dataset(dataset),
    point(RootPointPolicy::ChooseRoot(dataset)),
    scale(INT_MAX),
    base(base),
    parent(NULL),
    parentDistance(0),
    furthestDescendantDistance(0),
    localMetric(metric == NULL),
    metric(metric)
{
  // If we need to create a metric, do that.  We'll just do it on the heap.
  if (localMetric)
    this->metric = new MetricType();

  // If there is only one point in the dataset... uh, we're done.
  if (dataset.n_cols == 1)
    return;

  // Kick off the building.  Create the indices array and the distances array.
  arma::Col<size_t> indices = arma::linspace<arma::Col<size_t> >(1,
      dataset.n_cols - 1, dataset.n_cols - 1);
  // This is now [1 2 3 4 ... n].  We must be sure that our point does not
  // occur.
  if (point != 0)
    indices[point - 1] = 0; // Put 0 back into the set; remove what was there.

  arma::vec distances(dataset.n_cols - 1);

  // Build the initial distances.
  ComputeDistances(point, indices, distances, dataset.n_cols - 1);

  // Create the children.
  size_t farSetSize = 0;
  size_t usedSetSize = 0;
  CreateChildren(indices, distances, dataset.n_cols - 1, farSetSize,
      usedSetSize);

  // Use the furthest descendant distance to determine the scale of the root
  // node.
  scale = (int) ceil(log(furthestDescendantDistance) / log(base));

  // Initialize statistic.
  stat = StatisticType(*this);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::CoverTree(
    const arma::mat& dataset,
    MetricType& metric,
    const double base) :
    dataset(dataset),
    point(RootPointPolicy::ChooseRoot(dataset)),
    scale(INT_MAX),
    base(base),
    parent(NULL),
    parentDistance(0),
    furthestDescendantDistance(0),
    localMetric(false),
    metric(&metric)
{
  // If there is only one point in the dataset, uh, we're done.
  if (dataset.n_cols == 1)
    return;

  // Kick off the building.  Create the indices array and the distances array.
  arma::Col<size_t> indices = arma::linspace<arma::Col<size_t> >(1,
      dataset.n_cols - 1, dataset.n_cols - 1);
  // This is now [1 2 3 4 ... n].  We must be sure that our point does not
  // occur.
  if (point != 0)
    indices[point - 1] = 0; // Put 0 back into the set; remove what was there.

  arma::vec distances(dataset.n_cols - 1);

  // Build the initial distances.
  ComputeDistances(point, indices, distances, dataset.n_cols - 1);

  // Create the children.
  size_t farSetSize = 0;
  size_t usedSetSize = 0;
  CreateChildren(indices, distances, dataset.n_cols - 1, farSetSize,
      usedSetSize);

  // Use the furthest descendant distance to determine the scale of the root
  // node.
  scale = (int) ceil(log(furthestDescendantDistance) / log(base));

  // Initialize statistic.
  stat = StatisticType(*this);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::CoverTree(
    const arma::mat& dataset,
    const double base,
    const size_t pointIndex,
    const int scale,
    CoverTree* parent,
    const double parentDistance,
    arma::Col<size_t>& indices,
    arma::vec& distances,
    size_t nearSetSize,
    size_t& farSetSize,
    size_t& usedSetSize,
    MetricType& metric) :
    dataset(dataset),
    point(pointIndex),
    scale(scale),
    base(base),
    parent(parent),
    parentDistance(parentDistance),
    furthestDescendantDistance(0),
    localMetric(false),
    metric(&metric)
{
  // If the size of the near set is 0, this is a leaf.
  if (nearSetSize == 0)
  {
    this->scale = INT_MIN;
    stat = StatisticType(*this);
    return;
  }

  // Otherwise, create the children.
  CreateChildren(indices, distances, nearSetSize, farSetSize, usedSetSize);

  // Initialize statistic.
  stat = StatisticType(*this);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
inline void
CoverTree<MetricType, RootPointPolicy, StatisticType>::CreateChildren(
    arma::Col<size_t>& indices,
    arma::vec& distances,
    size_t nearSetSize,
    size_t& farSetSize,
    size_t& usedSetSize)
{
  // Determine the next scale level.  This should be the first level where there
  // are any points in the far set.  So, if we know the maximum distance in the
  // distances array, this will be the largest i such that
  //   maxDistance > pow(base, i)
  // and using this for the scale factor should guarantee we are not creating an
  // implicit node.  If the maximum distance is 0, every point in the near set
  // will be created as a leaf, and a child to this node.  We also do not need
  // to change the furthestChildDistance or furthestDescendantDistance.
  const double maxDistance = max(distances.rows(0,
      nearSetSize + farSetSize - 1));
  if (maxDistance == 0)
  {
    // Make the self child at the lowest possible level.
    // This should not modify farSetSize or usedSetSize.
    size_t tempSize = 0;
    children.push_back(new CoverTree(dataset, base, point, INT_MIN, this, 0,
        indices, distances, 0, tempSize, usedSetSize, *metric));

    // Every point in the near set should be a leaf.
    for (size_t i = 0; i < nearSetSize; ++i)
    {
      // farSetSize and usedSetSize will not be modified.
      children.push_back(new CoverTree(dataset, base, indices[i],
          INT_MIN, this, distances[i], indices, distances, 0, tempSize,
          usedSetSize, *metric));
      usedSetSize++;
    }

    // Re-sort the dataset.  We have
    // [ used | far | other used ]
    // and we want
    // [ far | all used ].
    SortPointSet(indices, distances, 0, usedSetSize, farSetSize);

    // Initialize the statistic.
    stat = StatisticType(*this);

    return;
  }

  const int nextScale = std::min(scale,
      (int) ceil(log(maxDistance) / log(base))) - 1;
  const double bound = pow(base, nextScale);

  // First, make the self child.  We must split the given near set into the near
  // set and far set for the self child.
  size_t childNearSetSize =
      SplitNearFar(indices, distances, bound, nearSetSize);

  // Build the self child (recursively).
  size_t childFarSetSize = nearSetSize - childNearSetSize;
  size_t childUsedSetSize = 0;
  children.push_back(new CoverTree(dataset, base, point, nextScale, this, 0,
      indices, distances, childNearSetSize, childFarSetSize, childUsedSetSize,
      *metric));

  // The self-child can't modify the furthestChildDistance away from 0, but it
  // can modify the furthestDescendantDistance.
  furthestDescendantDistance = children[0]->FurthestDescendantDistance();

  // If we created an implicit node, take its self-child instead (this could
  // happen multiple times).
  while (children[children.size() - 1]->NumChildren() == 1)
  {
    CoverTree* old = children[children.size() - 1];
    children.erase(children.begin() + children.size() - 1);

    // Now take its child.
    children.push_back(&(old->Child(0)));

    // Set its parent correctly.
    old->Child(0).Parent() = this;

    // Remove its child (so it doesn't delete it).
    old->Children().erase(old->Children().begin() + old->Children().size() - 1);

    // Now delete it.
    delete old;
  }

  // Now the arrays, in memory, look like this:
  // [ childFar | childUsed | far | used ]
  // but we need to move the used points past our far set:
  // [ childFar | far | childUsed + used ]
  // and keeping in mind that childFar = our near set,
  // [ near | far | childUsed + used ]
  // is what we are trying to make.
  SortPointSet(indices, distances, childFarSetSize, childUsedSetSize,
      farSetSize);

  // Update size of near set and used set.
  nearSetSize -= childUsedSetSize;
  usedSetSize += childUsedSetSize;

  // Now for each point in the near set, we need to make children.  To save
  // computation later, we'll create an array holding the points in the near
  // set, and then after each run we'll check which of those (if any) were used
  // and we will remove them.  ...if that's faster.  I think it is.
  while (nearSetSize > 0)
  {
    size_t newPointIndex = nearSetSize - 1;

    // Swap to front if necessary.
    if (newPointIndex != 0)
    {
      const size_t tempIndex = indices[newPointIndex];
      const double tempDist = distances[newPointIndex];

      indices[newPointIndex] = indices[0];
      distances[newPointIndex] = distances[0];

      indices[0] = tempIndex;
      distances[0] = tempDist;
    }

    // Will this be a new furthest child?
    if (distances[0] > furthestDescendantDistance)
      furthestDescendantDistance = distances[0];

    // If there's only one point left, we don't need this crap.
    if ((nearSetSize == 1) && (farSetSize == 0))
    {
      size_t childNearSetSize = 0;
      children.push_back(new CoverTree(dataset, base, indices[0], nextScale,
          this, distances[0], indices, distances, childNearSetSize, farSetSize,
          usedSetSize, *metric));

      // Because the far set size is 0, we don't have to do any swapping to
      // move the point into the used set.
      ++usedSetSize;
      --nearSetSize;

      // And we're done.
      break;
    }

    // Create the near and far set indices and distance vectors.  We don't fill
    // in the self-point, yet.
    arma::Col<size_t> childIndices(nearSetSize + farSetSize);
    childIndices.rows(0, (nearSetSize + farSetSize - 2)) = indices.rows(1,
        nearSetSize + farSetSize - 1);
    arma::vec childDistances(nearSetSize + farSetSize);

    // Build distances for the child.
    ComputeDistances(indices[0], childIndices, childDistances, nearSetSize
        + farSetSize - 1);

    // Split into near and far sets for this point.
    childNearSetSize = SplitNearFar(childIndices, childDistances, bound,
        nearSetSize + farSetSize - 1);
    childFarSetSize = PruneFarSet(childIndices, childDistances,
        base * bound, childNearSetSize,
        (nearSetSize + farSetSize - 1));

    // Now that we know the near and far set sizes, we can put the used point
    // (the self point) in the correct place; now, when we call
    // MoveToUsedSet(), it will move the self-point correctly.  The distance
    // does not matter.
    childIndices(childNearSetSize + childFarSetSize) = indices[0];
    childDistances(childNearSetSize + childFarSetSize) = 0;

    // Build this child (recursively).
    childUsedSetSize = 1; // Mark self point as used.
    children.push_back(new CoverTree(dataset, base, indices[0], nextScale,
        this, distances[0], childIndices, childDistances, childNearSetSize,
        childFarSetSize, childUsedSetSize, *metric));

    // If we created an implicit node, take its self-child instead (this could
    // happen multiple times).
    while (children[children.size() - 1]->NumChildren() == 1)
    {
      CoverTree* old = children[children.size() - 1];
      children.erase(children.begin() + children.size() - 1);

      // Now take its child.
      children.push_back(&(old->Child(0)));

      // Set its parent correctly.
      old->Child(0).Parent() = this;

      // Remove its child (so it doesn't delete it).
      old->Children().erase(old->Children().begin() + old->Children().size()
          - 1);

      // Now delete it.
      delete old;
    }

    // Now with the child created, it returns the childIndices and
    // childDistances vectors in this form:
    // [ childFar | childUsed ]
    // For each point in the childUsed set, we must move that point to the used
    // set in our own vector.
    MoveToUsedSet(indices, distances, nearSetSize, farSetSize, usedSetSize,
        childIndices, childFarSetSize, childUsedSetSize);
  }

  // Calculate furthest descendant.
  for (size_t i = (nearSetSize + farSetSize); i < (nearSetSize + farSetSize +
      usedSetSize); ++i)
    if (distances[i] > furthestDescendantDistance)
      furthestDescendantDistance = distances[i];
}

// Manually create a cover tree node.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::CoverTree(
    const arma::mat& dataset,
    const double base,
    const size_t pointIndex,
    const int scale,
    CoverTree* parent,
    const double parentDistance,
    const double furthestDescendantDistance,
    MetricType* metric) :
    dataset(dataset),
    point(pointIndex),
    scale(scale),
    base(base),
    parent(parent),
    parentDistance(parentDistance),
    furthestDescendantDistance(furthestDescendantDistance),
    localMetric(metric == NULL),
    metric(metric)
{
  // If necessary, create a local metric.
  if (localMetric)
    this->metric = new MetricType();

  // Initialize the statistic.
  stat = StatisticType(*this);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::CoverTree(
    const CoverTree& other) :
    dataset(other.dataset),
    point(other.point),
    scale(other.scale),
    base(other.base),
    stat(other.stat),
    parent(other.parent),
    parentDistance(other.parentDistance),
    furthestDescendantDistance(other.furthestDescendantDistance),
    localMetric(false),
    metric(other.metric)
{
  // Copy each child by hand.
  for (size_t i = 0; i < other.NumChildren(); ++i)
  {
    children.push_back(new CoverTree(other.Child(i)));
    children[i]->Parent() = this;
  }
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
CoverTree<MetricType, RootPointPolicy, StatisticType>::~CoverTree()
{
  // Delete each child.
  for (size_t i = 0; i < children.size(); ++i)
    delete children[i];

  // Delete the local metric, if necessary.
  if (localMetric)
    delete metric;
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MinDistance(
    const CoverTree<MetricType, RootPointPolicy, StatisticType>* other) const
{
  // Every cover tree node will contain points up to EC^(scale + 1) away.
  return std::max(metric->Evaluate(dataset.unsafe_col(point),
      other->Dataset().unsafe_col(other->Point())) -
      furthestDescendantDistance - other->FurthestDescendantDistance(), 0.0);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MinDistance(
    const CoverTree<MetricType, RootPointPolicy, StatisticType>* other,
    const double distance) const
{
  // We already have the distance as evaluated by the metric.
  return std::max(distance - furthestDescendantDistance -
      other->FurthestDescendantDistance(), 0.0);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MinDistance(
    const arma::vec& other) const
{
  return std::max(metric->Evaluate(dataset.unsafe_col(point), other) -
      furthestDescendantDistance, 0.0);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MinDistance(
    const arma::vec& /* other */,
    const double distance) const
{
  return std::max(distance - furthestDescendantDistance, 0.0);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MaxDistance(
    const CoverTree<MetricType, RootPointPolicy, StatisticType>* other) const
{
  return metric->Evaluate(dataset.unsafe_col(point),
      other->Dataset().unsafe_col(other->Point())) +
      furthestDescendantDistance + other->FurthestDescendantDistance();
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MaxDistance(
    const CoverTree<MetricType, RootPointPolicy, StatisticType>* other,
    const double distance) const
{
  // We already have the distance as evaluated by the metric.
  return distance + furthestDescendantDistance +
      other->FurthestDescendantDistance();
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MaxDistance(
    const arma::vec& other) const
{
  return metric->Evaluate(dataset.unsafe_col(point), other) +
      furthestDescendantDistance;
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::MaxDistance(
    const arma::vec& /* other */,
    const double distance) const
{
  return distance + furthestDescendantDistance;
}

//! Return the minimum and maximum distance to another node.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::RangeDistance(
    const CoverTree* other) const
{
  const double distance = metric->Evaluate(dataset.unsafe_col(point),
      other->Dataset().unsafe_col(other->Point()));

  math::Range result;
  result.Lo() = distance - furthestDescendantDistance -
      other->FurthestDescendantDistance();
  result.Hi() = distance + furthestDescendantDistance +
      other->FurthestDescendantDistance();

  return result;
}

//! Return the minimum and maximum distance to another node given that the
//! point-to-point distance has already been calculated.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::RangeDistance(
    const CoverTree* other,
    const double distance) const
{
  math::Range result;
  result.Lo() = distance - furthestDescendantDistance -
      other->FurthestDescendantDistance();
  result.Hi() = distance + furthestDescendantDistance +
      other->FurthestDescendantDistance();

  return result;
}

//! Return the minimum and maximum distance to another point.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::RangeDistance(
    const arma::vec& other) const
{
  const double distance = metric->Evaluate(dataset.unsafe_col(point), other);

  return math::Range(distance - furthestDescendantDistance,
                     distance + furthestDescendantDistance);
}

//! Return the minimum and maximum distance to another point given that the
//! point-to-point distance has already been calculated.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
double CoverTree<MetricType, RootPointPolicy, StatisticType>::RangeDistance(
    const arma::vec& other,
    const double distance) const
{
  return math::Range(distance - furthestDescendantDistance,
                     distance + furthestDescendantDistance);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
size_t CoverTree<MetricType, RootPointPolicy, StatisticType>::SplitNearFar(
    arma::Col<size_t>& indices,
    arma::vec& distances,
    const double bound,
    const size_t pointSetSize)
{
  // Sanity check; there is no guarantee that this condition will not be true.
  // ...or is there?
  if (pointSetSize <= 1)
    return 0;

  // We'll traverse from both left and right.
  size_t left = 0;
  size_t right = pointSetSize - 1;

  // A modification of quicksort, with the pivot value set to the bound.
  // Everything on the left of the pivot will be less than or equal to the
  // bound; everything on the right will be greater than the bound.
  while ((distances[left] <= bound) && (left != right))
    ++left;
  while ((distances[right] > bound) && (left != right))
    --right;

  while (left != right)
  {
    // Now swap the values and indices.
    const size_t tempPoint = indices[left];
    const double tempDist = distances[left];

    indices[left] = indices[right];
    distances[left] = distances[right];

    indices[right] = tempPoint;
    distances[right] = tempDist;

    // Traverse the left, seeing how many points are correctly on that side.
    // When we encounter an incorrect point, stop.  We will switch it later.
    while ((distances[left] <= bound) && (left != right))
      ++left;

    // Traverse the right, seeing how many points are correctly on that side.
    // When we encounter an incorrect point, stop.  We will switch it with the
    // wrong point from the left side.
    while ((distances[right] > bound) && (left != right))
      --right;
  }

  // The final left value is the index of the first far value.
  return left;
}

// Returns the maximum distance between points.
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
void CoverTree<MetricType, RootPointPolicy, StatisticType>::ComputeDistances(
    const size_t pointIndex,
    const arma::Col<size_t>& indices,
    arma::vec& distances,
    const size_t pointSetSize)
{
  // For each point, rebuild the distances.  The indices do not need to be
  // modified.
  for (size_t i = 0; i < pointSetSize; ++i)
  {
    distances[i] = metric->Evaluate(dataset.unsafe_col(pointIndex),
        dataset.unsafe_col(indices[i]));
  }
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
size_t CoverTree<MetricType, RootPointPolicy, StatisticType>::SortPointSet(
    arma::Col<size_t>& indices,
    arma::vec& distances,
    const size_t childFarSetSize,
    const size_t childUsedSetSize,
    const size_t farSetSize)
{
  // We'll use low-level memcpy calls ourselves, just to ensure it's done
  // quickly and the way we want it to be.  Unfortunately this takes up more
  // memory than one-element swaps, but there's not a great way around that.
  const size_t bufferSize = std::min(farSetSize, childUsedSetSize);
  const size_t bigCopySize = std::max(farSetSize, childUsedSetSize);

  // Sanity check: there is no need to sort if the buffer size is going to be
  // zero.
  if (bufferSize == 0)
    return (childFarSetSize + farSetSize);

  size_t* indicesBuffer = new size_t[bufferSize];
  double* distancesBuffer = new double[bufferSize];

  // The start of the memory region to copy to the buffer.
  const size_t bufferFromLocation = ((bufferSize == farSetSize) ?
      (childFarSetSize + childUsedSetSize) : childFarSetSize);
  // The start of the memory region to move directly to the new place.
  const size_t directFromLocation = ((bufferSize == farSetSize) ?
      childFarSetSize : (childFarSetSize + childUsedSetSize));
  // The destination to copy the buffer back to.
  const size_t bufferToLocation = ((bufferSize == farSetSize) ?
      childFarSetSize : (childFarSetSize + farSetSize));
  // The destination of the directly moved memory region.
  const size_t directToLocation = ((bufferSize == farSetSize) ?
      (childFarSetSize + farSetSize) : childFarSetSize);

  // Copy the smaller piece to the buffer.
  memcpy(indicesBuffer, indices.memptr() + bufferFromLocation,
      sizeof(size_t) * bufferSize);
  memcpy(distancesBuffer, distances.memptr() + bufferFromLocation,
      sizeof(double) * bufferSize);

  // Now move the other memory.
  memmove(indices.memptr() + directToLocation,
      indices.memptr() + directFromLocation, sizeof(size_t) * bigCopySize);
  memmove(distances.memptr() + directToLocation,
      distances.memptr() + directFromLocation, sizeof(double) * bigCopySize);

  // Now copy the temporary memory to the right place.
  memcpy(indices.memptr() + bufferToLocation, indicesBuffer,
      sizeof(size_t) * bufferSize);
  memcpy(distances.memptr() + bufferToLocation, distancesBuffer,
      sizeof(double) * bufferSize);

  delete[] indicesBuffer;
  delete[] distancesBuffer;

  // This returns the complete size of the far set.
  return (childFarSetSize + farSetSize);
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
void CoverTree<MetricType, RootPointPolicy, StatisticType>::MoveToUsedSet(
    arma::Col<size_t>& indices,
    arma::vec& distances,
    size_t& nearSetSize,
    size_t& farSetSize,
    size_t& usedSetSize,
    arma::Col<size_t>& childIndices,
    const size_t childFarSetSize, // childNearSetSize is 0 in this case.
    const size_t childUsedSetSize)
{
  const size_t originalSum = nearSetSize + farSetSize + usedSetSize;

  // Loop across the set.  We will swap points as we need.  It should be noted
  // that farSetSize and nearSetSize may change with each iteration of this loop
  // (depending on if we make a swap or not).
  size_t startChildUsedSet = 0; // Where to start in the child set.
  for (size_t i = 0; i < nearSetSize; ++i)
  {
    // Discover if this point was in the child's used set.
    for (size_t j = startChildUsedSet; j < childUsedSetSize; ++j)
    {
      if (childIndices[childFarSetSize + j] == indices[i])
      {
        // We have found a point; a swap is necessary.

        // Since this point is from the near set, to preserve the near set, we
        // must do a swap.
        if (farSetSize > 0)
        {
          if ((nearSetSize - 1) != i)
          {
            // In this case it must be a three-way swap.
            size_t tempIndex = indices[nearSetSize + farSetSize - 1];
            double tempDist = distances[nearSetSize + farSetSize - 1];

            size_t tempNearIndex = indices[nearSetSize - 1];
            double tempNearDist = distances[nearSetSize - 1];

            indices[nearSetSize + farSetSize - 1] = indices[i];
            distances[nearSetSize + farSetSize - 1] = distances[i];

            indices[nearSetSize - 1] = tempIndex;
            distances[nearSetSize - 1] = tempDist;

            indices[i] = tempNearIndex;
            distances[i] = tempNearDist;
          }
          else
          {
            // We can do a two-way swap.
            size_t tempIndex = indices[nearSetSize + farSetSize - 1];
            double tempDist = distances[nearSetSize + farSetSize - 1];

            indices[nearSetSize + farSetSize - 1] = indices[i];
            distances[nearSetSize + farSetSize - 1] = distances[i];

            indices[i] = tempIndex;
            distances[i] = tempDist;
          }
        }
        else if ((nearSetSize - 1) != i)
        {
          // A two-way swap is possible.
          size_t tempIndex = indices[nearSetSize + farSetSize - 1];
          double tempDist = distances[nearSetSize + farSetSize - 1];

          indices[nearSetSize + farSetSize - 1] = indices[i];
          distances[nearSetSize + farSetSize - 1] = distances[i];

          indices[i] = tempIndex;
          distances[i] = tempDist;
        }
        else
        {
          // No swap is necessary.
        }

        // We don't need to do a complete preservation of the child index set,
        // but we want to make sure we only loop over points we haven't seen.
        // So increment the child counter by 1 and move a point if we need.
        if (j != startChildUsedSet)
        {
          childIndices[childFarSetSize + j] = childIndices[childFarSetSize +
              startChildUsedSet];
        }

        // Update all counters from the swaps we have done.
        ++startChildUsedSet;
        --nearSetSize;
        --i; // Since we moved a point out of the near set we must step back.

        break; // Break out of this for loop; back to the first one.
      }
    }
  }

  // Now loop over the far set.  This loop is different because we only require
  // a normal two-way swap instead of the three-way swap to preserve the near
  // set / far set ordering.
  for (size_t i = 0; i < farSetSize; ++i)
  {
    // Discover if this point was in the child's used set.
    for (size_t j = startChildUsedSet; j < childUsedSetSize; ++j)
    {
      if (childIndices[childFarSetSize + j] == indices[i + nearSetSize])
      {
        // We have found a point to swap.

        // Perform the swap.
        size_t tempIndex = indices[nearSetSize + farSetSize - 1];
        double tempDist = distances[nearSetSize + farSetSize - 1];

        indices[nearSetSize + farSetSize - 1] = indices[nearSetSize + i];
        distances[nearSetSize + farSetSize - 1] = distances[nearSetSize + i];

        indices[nearSetSize + i] = tempIndex;
        distances[nearSetSize + i] = tempDist;

        if (j != startChildUsedSet)
        {
          childIndices[childFarSetSize + j] = childIndices[childFarSetSize +
              startChildUsedSet];
        }

        // Update all counters from the swaps we have done.
        ++startChildUsedSet;
        --farSetSize;
        --i;

        break; // Break out of this for loop; back to the first one.
      }
    }
  }

  // Update used set size.
  usedSetSize += childUsedSetSize;

  Log::Assert(originalSum == (nearSetSize + farSetSize + usedSetSize));
}

template<typename MetricType, typename RootPointPolicy, typename StatisticType>
size_t CoverTree<MetricType, RootPointPolicy, StatisticType>::PruneFarSet(
    arma::Col<size_t>& indices,
    arma::vec& distances,
    const double bound,
    const size_t nearSetSize,
    const size_t pointSetSize)
{
  // What we are trying to do is remove any points greater than the bound from
  // the far set.  We don't care what happens to those indices and distances...
  // so, we don't need to properly swap points -- just drop new ones in place.
  size_t left = nearSetSize;
  size_t right = pointSetSize - 1;
  while ((distances[left] <= bound) && (left != right))
    ++left;
  while ((distances[right] > bound) && (left != right))
    --right;

  while (left != right)
  {
    // We don't care what happens to the point which should be on the right.
    indices[left] = indices[right];
    distances[left] = distances[right];
    --right; // Since we aren't changing the right.

    // Advance to next location which needs to switch.
    while ((distances[left] <= bound) && (left != right))
      ++left;
    while ((distances[right] > bound) && (left != right))
      --right;
  }

  // The far set size is the left pointer, with the near set size subtracted
  // from it.
  return (left - nearSetSize);
}

/**
 * Returns a string representation of this object.
 */
template<typename MetricType, typename RootPointPolicy, typename StatisticType>
std::string CoverTree<MetricType, RootPointPolicy, StatisticType>::ToString() const
{
  std::ostringstream convert;
  convert << "CoverTree [" << this << "]" << std::endl;
  convert << "dataset: " << &dataset << std::endl;
  convert << "point: " << point << std::endl;
  convert << "scale: " << scale << std::endl;
  convert << "base: " << base << std::endl;
//  convert << "StatisticType: " << stat << std::endl;
  convert << "parent distance : " << parentDistance << std::endl;
  convert << "furthest child distance: " << furthestDescendantDistance;
  convert << std::endl;
  convert << "children:";

  if (IsLeaf() == false)
  {
    for (int i = 0; i < children.size(); i++)
    {
      convert << std::endl << mlpack::util::Indent(children.at(i)->ToString());
    }
  }
  return convert.str();
}
}; // namespace tree
}; // namespace mlpack

#endif
