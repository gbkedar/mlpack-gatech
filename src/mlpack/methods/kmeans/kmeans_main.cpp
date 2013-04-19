/**
 * @file kmeans_main.cpp
 * @author Ryan Curtin
 *
 * Executable for running K-Means.
 */
#include <mlpack/core.hpp>

#include "kmeans.hpp"
#include "allow_empty_clusters.hpp"
#include "refined_start.hpp"

using namespace mlpack;
using namespace mlpack::kmeans;
using namespace std;

// Define parameters for the executable.
PROGRAM_INFO("K-Means Clustering", "This program performs K-Means clustering "
    "on the given dataset, storing the learned cluster assignments either as "
    "a column of labels in the file containing the input dataset or in a "
    "separate file.  Empty clusters are not allowed by default; when a cluster "
    "becomes empty, the point furthest from the centroid of the cluster with "
    "maximum variance is taken to fill that cluster."
    "\n\n"
    "Optionally, the Bradley and Fayyad approach (\"Refining initial points for"
    " k-means clustering\", 1998) can be used to select initial points by "
    "specifying the --refined_start (-r) option.  This approach works by taking"
    " random samples of the dataset; to specify the number of samples, the "
    "--samples parameter is used, and to specify the percentage of the dataset "
    "to be used in each sample, the --percentage parameter is used (it should "
    "be a value between 0.0 and 1.0)."
    "\n\n"
    "If you want to specify your own initial cluster assignments or initial "
    "cluster centroids, this functionality is available in the C++ interface. "
    "Alternately, file a bug (well, a feature request) on the mlpack bug "
    "tracker.");

// Required options.
PARAM_STRING_REQ("inputFile", "Input dataset to perform clustering on.", "i");
PARAM_INT_REQ("clusters", "Number of clusters to find.", "c");

// Output options.
PARAM_FLAG("in_place", "If specified, a column of the learned cluster "
    "assignments will be added to the input dataset file.  In this case, "
    "--outputFile is not necessary.", "p");
PARAM_STRING("output_file", "File to write output labels or labeled data to.",
    "o", "output.csv");
PARAM_STRING("centroid_file", "If specified, the centroids of each cluster will"
    " be written to the given file.", "C", "");

// k-means configuration options.
PARAM_FLAG("allow_empty_clusters", "Allow empty clusters to be created.", "e");
PARAM_FLAG("labels_only", "Only output labels into output file.", "l");
PARAM_DOUBLE("overclustering", "Finds (overclustering * clusters) clusters, "
    "then merges them together until only the desired number of clusters are "
    "left.", "O", 1.0);
PARAM_INT("max_iterations", "Maximum number of iterations before K-Means "
    "terminates.", "m", 1000);
PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);

// This is known to not work (#251).
//PARAM_FLAG("fast_kmeans", "Use the experimental fast k-means algorithm by "
//    "Pelleg and Moore.", "f");

// Parameters for "refined start" k-means.
PARAM_FLAG("refined_start", "Use the refined initial point strategy by Bradley "
    "and Fayyad to choose initial points.", "r");
PARAM_INT("samplings", "Number of samplings to perform for refined start (use "
    "when --refined_start is specified).", "S", 100);
PARAM_DOUBLE("percentage", "Percentage of dataset to use for each refined start"
    " sampling (use when --refined_start is specified).", "p", 0.02);


int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);

  // Initialize random seed.
  if (CLI::GetParam<int>("seed") != 0)
    math::RandomSeed((size_t) CLI::GetParam<int>("seed"));
  else
    math::RandomSeed((size_t) std::time(NULL));

  // Now do validation of options.
  string inputFile = CLI::GetParam<string>("inputFile");
  int clusters = CLI::GetParam<int>("clusters");
  if (clusters < 1)
  {
    Log::Fatal << "Invalid number of clusters requested (" << clusters << ")! "
        << "Must be greater than or equal to 1." << std::endl;
  }

  int maxIterations = CLI::GetParam<int>("max_iterations");
  if (maxIterations < 0)
  {
    Log::Fatal << "Invalid value for maximum iterations (" << maxIterations <<
        ")! Must be greater than or equal to 0." << std::endl;
  }

  double overclustering = CLI::GetParam<double>("overclustering");
  if (overclustering < 1)
  {
    Log::Fatal << "Invalid value for overclustering (" << overclustering <<
        ")! Must be greater than or equal to 1." << std::endl;
  }

  // Make sure we have an output file if we're not doing the work in-place.
  if (!CLI::HasParam("in_place") && !CLI::HasParam("output_file"))
  {
    Log::Fatal << "--outputFile not specified (and --in_place not set)."
        << std::endl;
  }

  // Load our dataset.
  arma::mat dataset;
  data::Load(inputFile.c_str(), dataset);

  // Now create the KMeans object.  Because we could be using different types,
  // it gets a little weird...
  arma::Col<size_t> assignments;
  arma::mat centroids;

  if (CLI::HasParam("allow_empty_clusters"))
  {
    if (CLI::HasParam("refined_start"))
    {
      const int samplings = CLI::GetParam<int>("samplings");
      const double percentage = CLI::GetParam<int>("percentage");

      if (samplings < 0)
        Log::Fatal << "Number of samplings (" << samplings << ") must be "
            << "greater than 0!" << std::endl;
      if (percentage <= 0.0 || percentage > 1.0)
        Log::Fatal << "Percentage for sampling (" << percentage << ") must be "
            << "greater than 0.0 and less than or equal to 1.0!" << std::endl;

      KMeans<metric::SquaredEuclideanDistance, RefinedStart, AllowEmptyClusters>
          k(maxIterations, overclustering, metric::SquaredEuclideanDistance(),
          RefinedStart(samplings, percentage));

      Timer::Start("clustering");
      if (CLI::HasParam("fast_kmeans"))
        k.FastCluster(dataset, clusters, assignments);
      else
        k.Cluster(dataset, clusters, assignments, centroids);
      Timer::Stop("clustering");
    }
    else
    {
      KMeans<metric::SquaredEuclideanDistance, RandomPartition,
          AllowEmptyClusters> k(maxIterations, overclustering);

      Timer::Start("clustering");
      if (CLI::HasParam("fast_kmeans"))
        k.FastCluster(dataset, clusters, assignments);
      else
        k.Cluster(dataset, clusters, assignments, centroids);
      Timer::Stop("clustering");
    }
  }
  else
  {
    if (CLI::HasParam("refined_start"))
    {
      const int samplings = CLI::GetParam<int>("samplings");
      const double percentage = CLI::GetParam<int>("percentage");

      if (samplings < 0)
        Log::Fatal << "Number of samplings (" << samplings << ") must be "
            << "greater than 0!" << std::endl;
      if (percentage <= 0.0 || percentage > 1.0)
        Log::Fatal << "Percentage for sampling (" << percentage << ") must be "
            << "greater than 0.0 and less than or equal to 1.0!" << std::endl;

      KMeans<metric::SquaredEuclideanDistance, RefinedStart, AllowEmptyClusters>
          k(maxIterations, overclustering, metric::SquaredEuclideanDistance(),
          RefinedStart(samplings, percentage));

      Timer::Start("clustering");
      if (CLI::HasParam("fast_kmeans"))
        k.FastCluster(dataset, clusters, assignments);
      else
        k.Cluster(dataset, clusters, assignments, centroids);
      Timer::Stop("clustering");
    }
    else
    {
      KMeans<> k(maxIterations, overclustering);

      Timer::Start("clustering");
      if (CLI::HasParam("fast_kmeans"))
        k.FastCluster(dataset, clusters, assignments);
      else
        k.Cluster(dataset, clusters, assignments, centroids);
      Timer::Stop("clustering");
    }
  }

  // Now figure out what to do with our results.
  if (CLI::HasParam("in_place"))
  {
    // Add the column of assignments to the dataset; but we have to convert them
    // to type double first.
    arma::vec converted(assignments.n_elem);
    for (size_t i = 0; i < assignments.n_elem; i++)
      converted(i) = (double) assignments(i);

    dataset.insert_rows(dataset.n_rows, trans(converted));

    // Save the dataset.
    data::Save(inputFile, dataset);
  }
  else
  {
    if (CLI::HasParam("labels_only"))
    {
      // Save only the labels.
      string outputFile = CLI::GetParam<string>("outputFile");
      arma::Mat<size_t> output = trans(assignments);
      data::Save(outputFile.c_str(), output);
    }
    else
    {
      // Convert the assignments to doubles.
      arma::vec converted(assignments.n_elem);
      for (size_t i = 0; i < assignments.n_elem; i++)
        converted(i) = (double) assignments(i);

      dataset.insert_rows(dataset.n_rows, trans(converted));

      // Now save, in the different file.
      string outputFile = CLI::GetParam<string>("outputFile");
      data::Save(outputFile, dataset);
    }
  }

  // Should we write the centroids to a file?
  if (CLI::HasParam("centroids_file"))
    data::Save(CLI::GetParam<std::string>("centroids_file"), centroids);
}

