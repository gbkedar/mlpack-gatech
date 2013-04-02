/**
 * @author Parikshit Ram (pram@cc.gatech.edu)
 * @file gmm_main.cpp
 *
 * This program trains a mixture of Gaussians on a given data matrix.
 */
#include "gmm.hpp"

PROGRAM_INFO("Gaussian Mixture Model (GMM) Training",
    "This program takes a parametric estimate of a Gaussian mixture model (GMM)"
    " using the EM algorithm to find the maximum likelihood estimate.  The "
    "model is saved to an XML file, which contains information about each "
    "Gaussian.");

PARAM_STRING_REQ("input_file", "File containing the data on which the model "
    "will be fit.", "i");
PARAM_INT("gaussians", "Number of Gaussians in the GMM.", "g", 1);
PARAM_STRING("output_file", "The file to write the trained GMM parameters into "
    "(as XML).", "o", "gmm.xml");
PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);

using namespace mlpack;
using namespace mlpack::gmm;
using namespace mlpack::util;

int main(int argc, char* argv[])
{
  CLI::ParseCommandLine(argc, argv);

  // Check parameters and load data.
  if (CLI::GetParam<int>("seed") != 0)
    math::RandomSeed((size_t) CLI::GetParam<int>("seed"));
  else
    math::RandomSeed((size_t) std::time(NULL));

  arma::mat dataPoints;
  data::Load(CLI::GetParam<std::string>("input_file").c_str(), dataPoints,
      true);

  int gaussians = CLI::GetParam<int>("gaussians");
  if (gaussians <= 0)
  {
    Log::Fatal << "Invalid number of Gaussians (" << gaussians << "); must "
        "be greater than or equal to 1." << std::endl;
  }

  // Calculate mixture of Gaussians.
  GMM<> gmm(size_t(gaussians), dataPoints.n_rows);

  // Compute the parameters of the model using the EM algorithm.
  Timer::Start("em");
  double likelihood = gmm.Estimate(dataPoints);
  Timer::Stop("em");

  Log::Info << "Log-likelihood of estimate: " << likelihood << ".\n";

  // Save results.
  gmm.Save(CLI::GetParam<std::string>("output_file"));
}
