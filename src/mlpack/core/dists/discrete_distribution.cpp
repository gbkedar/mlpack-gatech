/**
 * @file discrete_distribution.cpp
 * @author Ryan Curtin
 *
 * Implementation of DiscreteDistribution probability distribution.
 */
#include "discrete_distribution.hpp"

using namespace mlpack;
using namespace mlpack::distribution;

/**
 * Return a randomly generated observation according to the probability
 * distribution defined by this object.
 */
arma::vec DiscreteDistribution::Random() const
{
  // Generate a random number.
  double randObs = math::Random();
  arma::vec result(1);

  double sumProb = 0;
  for (size_t obs = 0; obs < probabilities.n_elem; obs++)
  {
    if ((sumProb += probabilities[obs]) >= randObs)
    {
      result[0] = obs;
      return result;
    }
  }

  // This shouldn't happen.
  result[0] = probabilities.n_elem - 1;
  return result;
}

/**
 * Estimate the probability distribution directly from the given observations.
 */
void DiscreteDistribution::Estimate(const arma::mat& observations)
{
  // Clear old probabilities.
  probabilities.zeros();

  // Add the probability of each observation.  The addition of 0.5 to the
  // observation is to turn the default flooring operation of the size_t cast
  // into a rounding operation.
  for (size_t i = 0; i < observations.n_cols; i++)
    probabilities((size_t) (observations(0, i) + 0.5))++;

  // Now normalize the distribution.
  double sum = accu(probabilities);
  if (sum > 0)
    probabilities /= sum;
  else
    probabilities.fill(1 / probabilities.n_elem); // Force normalization.
}

/**
 * Estimate the probability distribution from the given observations when also
 * given probabilities that each observation is from this distribution.
 */
void DiscreteDistribution::Estimate(const arma::mat& observations,
                                    const arma::vec& probObs)
{
  // Clear old probabilities.
  probabilities.zeros();

  // Add the probability of each observation.  The addition of 0.5 to the
  // observation is to turn the default flooring operation of the size_t cast
  // into a rounding observation.
  for (size_t i = 0; i < observations.n_cols; i++)
    probabilities((size_t) (observations(0, i) + 0.5)) += probObs[i];

  // Now normalize the distribution.
  double sum = accu(probabilities);
  if (sum > 0)
    probabilities /= sum;
  else
    probabilities.fill(1 / probabilities.n_elem); // Force normalization.
}
