/**
 * @file mrkd_statistic.cpp
 * @author James Cline
 *
 * Definition of the statistic for multi-resolution kd-trees.
 */
#include "mrkd_statistic.hpp"

using namespace mlpack;
using namespace mlpack::tree;

MRKDStatistic::MRKDStatistic() :
    dataset(NULL),
    begin(0),
    count(0),
    leftStat(NULL),
    rightStat(NULL),
    parentStat(NULL)
{ }

template<typename TreeType>
MRKDStatistic::MRKDStatistic(const TreeType& /* node */) :
    dataset(NULL),
    begin(0),
    count(0),
    leftStat(NULL),
    rightStat(NULL),
    parentStat(NULL)
{ }

/**
 * Returns a string representation of this object.
 */
std::string MRKDStatistic::ToString() const
{
  std::ostringstream convert;

  convert << "MRKDStatistic [" << this << std::endl;
  convert << "begin: " << begin << std::endl;
  convert << "count: " << count << std::endl;
  convert << "sumOfSquaredNorms: " << sumOfSquaredNorms << std::endl;
  if (leftStat != NULL)
  {
    convert << "leftStat:" << std::endl;
    convert << mlpack::util::Indent(leftStat->ToString());
  }
  if (rightStat != NULL)
  {
    convert << "rightStat:" << std::endl;
    convert << mlpack::util::Indent(rightStat->ToString());
  }
  return convert.str();
}
