
#ifndef FASTLIB_GAUSSIAN_DISTRIBUTCLIN_H
#define FASTLIB_GAUSSIAN_DISTRIBUTCLIN_H
#include <fastlib/fastlib.h>

class GaussianDistribution {
  Vector mean;
  Matrix covariance;
	
  double gConst;
  Matrix invCov;
  Matrix sqrCov;
	
  Vector accMean;
  Matrix accCov;
  double accDenom;
 public:
  GaussianDistribution(const Vector& mean, const Matrix& cov);
  GaussianDistribution(size_t dim = 1);
  GaussianDistribution(const GaussianDistribution& gd);
	
  double logP(const Vector& x);	
  static void createFromCols(const Matrix& src,
			     size_t col, GaussianDistribution* tmp);
  void Generate(Vector* x);
  void StartAccumulate();
  void EndAccumulate();
  void Accumulate(const Vector& x, double weight);
			
  void Save(FILE* f);	
  const Vector& getMean() { return mean; }
  const Matrix& getCov() { return covariance; }
  void InitMeanCov(size_t dim);
  void setMeanCov(const Vector& mean, const Matrix& cov);
  size_t n_dim() { return mean.length(); }
};

#endif
