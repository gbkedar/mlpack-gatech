/**
 * @file cosine_tree_builder_impl.hpp
 * @author Mudit Raj Gupta
 *
 * Implementation of cosine tree builder.
 */

#ifndef __MLPACK_CORE_TREE_COSINE_TREE_COSINE_TREE_BUILDER_IMPL_HPP
#define __MLPACK_CORE_TREE_COSINE_TREE_COSINE_TREE_BUILDER_IMPL_HPP

#include "cosine_tree_builder.hpp"

namespace mlpack {
namespace tree {

// Empty Constructor
CosineTreeBuilder::CosineTreeBuilder() 
{
  Log::Info<<"Constructor"<<std::endl;
}

// Destructor
CosineTreeBuilder::~CosineTreeBuilder() {}

void CosineTreeBuilder::LSSampling(arma::mat A, arma::vec& probability)
{
  Log::Info<<"LSSampling"<<std::endl;
  //Saving the frobenious norm of the matrix
  double normA = arma::norm(A,"fro");
  //Calculating probability of each point to be sampled
  for (size_t i=0;i<A.n_rows;i++)
    probability(i) = arma::norm(A.row(i),"fro")/normA;
}

double CosineTreeBuilder::EuclideanNorm(arma::vec A)
{
   Log::Info<<"EuclideanNorm"<<std::endl;
   //Calculating the Euclidean Norm
   return sqrt(arma::sum(arma::square(A)));
}

double CosineTreeBuilder::CosineSimilarity(arma::vec A, arma::vec B)
{
  Log::Info<<"CosineSimilarity"<<std::endl;
  //Value of the cosine
  double value = arma::dot(A,B)/(EuclideanNorm(A)*EuclideanNorm(B));
  //Stripping to account for floating point error
  if (value > 1.0)
    value = 1.0;
  if (value < -1.0)
    value = -1.0;
  //Calculating angle
  return acos(value);
}

arma::rowvec CosineTreeBuilder::CalculateCentroid(arma::mat A) const
{
  Log::Info<<"CalculateCentroid"<<std::endl;
  //Summing over all coloumns
  arma::rowvec colsum = arma::sum(A,0);
  //Averaging
  double k = 1.0/A.n_rows;
  return k*colsum;
}

void CosineTreeBuilder::CTNode(arma::mat A, CosineTree& root)
{
  Log::Info<<"CTNode"<<std::endl;
  //Calculating Centroid
  arma::rowvec centroid = CalculateCentroid(A);
  //Calculating sampling probabilities
  arma::vec probabilities = arma::zeros<arma::vec>(A.n_rows,1);
  LSSampling(A,probabilities);
  //Setting Values
  root.Probabilities(probabilities);
  root.Data(A);
  root.Centroid(centroid);
  root.Left(NULL);
  root.Right(NULL);
  root.NumPoints(A.n_rows);
}

size_t CosineTreeBuilder::GetPivot(arma::vec prob)
{
  Log::Info<<"GetPivot"<<std::endl;
  //Setting firtst value as the pivot
  double maxPivot=prob(0,0); 
  size_t pivot=0;
 
  //Searching for the pivot poitn
  for (size_t i=0;i<prob.n_rows;i++)
  {
    if(prob(i,0) > maxPivot)
    {
      maxPivot = prob(i,0);
      pivot = i;
    }
  }
  return pivot;
}

void CosineTreeBuilder::SplitData(std::vector<double> c, arma::mat& ALeft, 
                                  arma::mat& ARight,arma::mat A)
{
  Log::Info<<"SplitData"<<std::endl;
  double cMax,cMin;
  //Calculating the lower and the Upper Limit
  cMin = GetMaxSimilarity(c);
  cMax = GetMinSimilarity(c);
  //Couter for left and right
  size_t lft, rgt;
  lft = 0; 
  rgt = 0;
  //Splitting on the basis of nearness to the the high or low value
  for(size_t i=0;i<A.n_rows;i++)
  {
    if ((cMax - c[i])<=(c[i] - cMin))
    {
      ALeft.insert_rows(lft,A.row(i));
      lft ++;
    }
    else
    {
      ARight.insert_rows(rgt,A.row(i));
      rgt ++;
    }
  }
}

void CosineTreeBuilder::CreateCosineSimilarityArray(std::vector<double>& c, 
                                                    arma::mat A, size_t pivot)
{
  Log::Info<<"CreateCosineSimilarityArray"<<std::endl;
  for(size_t i=0;i<A.n_rows;i++)
    c.push_back(CosineSimilarity(A.row(pivot).t(),A.row(i).t()));
}
double CosineTreeBuilder::GetMinSimilarity(std::vector<double> c)
{
  Log::Info<<"GetMinSimilarity"<<std::endl;
  double cMin = c[0];
  for(size_t i=1;i<c.size();i++)
    if(cMin<c[i])  
      cMin = c[i];
  return cMin;
}
double CosineTreeBuilder::GetMaxSimilarity(std::vector<double> c)
{
  Log::Info<<"GetMaxSimilarity"<<std::endl;
  double cMax = c[0];
  for(size_t i=1;i<c.size();i++)
    if(cMax<c[i])  
      cMax = c[i];
  return cMax;
}
void CosineTreeBuilder::CTNodeSplit(CosineTree& root, CosineTree& left, 
                                    CosineTree& right)
{
  //Extracting points from the root
  arma::mat A = root.Data();
  //Cosine Similarity Array
  std::vector<double> c;
  //Matrices holding points for the left and the right node
  arma::mat ALeft, ARight;
  //Sampling probabilities
  arma::vec prob = root.Probabilities();
  //Pivot
  size_t pivot = GetPivot(prob);
  //Creating Array
  CreateCosineSimilarityArray(c,A,pivot);
  //Splitting data points
  SplitData(c,ALeft,ARight,A);
  //Creating Nodes
  if(ALeft.n_rows > 0)
  {
    CTNode(ALeft,left);
    //TODO: Traversal is not required, still fix this
    //root.Left(left);
  }
  if(ARight.n_rows > 0)
  {
    CTNode(ARight,right);
    //TODO: Traversal is not required, still fix this
    //root.Right(right);
  }
}

}; // namespace cosinetreebuilder
}; // namespace mlpack

#endif
