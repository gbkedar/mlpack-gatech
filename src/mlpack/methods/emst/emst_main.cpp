/**
 * @file emst_main.cpp
 * @author Bill March (march@gatech.edu)
 *
 * Calls the DualTreeBoruvka algorithm from dtb.hpp.
 * Can optionally call naive Boruvka's method.
 *
 * For algorithm details, see:
 * March, W.B., Ram, P., and Gray, A.G.
 * Fast Euclidean Minimum Spanning Tree: Algorithm, Analysis, Applications.
 * In KDD, 2010.
 */

#include "dtb.hpp"

#include <mlpack/core.hpp>

PROGRAM_INFO("Fast Euclidean Minimum Spanning Tree", "This program can compute "
    "the Euclidean minimum spanning tree of a set of input points using the "
    "dual-tree Boruvka algorithm.  This method is detailed in the following "
    "paper:\n\n"
    "  @inproceedings{\n"
    "    author = {March, W.B., Ram, P., and Gray, A.G.},\n"
    "    title = {{Fast Euclidean Minimum Spanning Tree: Algorithm, Analysis,\n"
    "        Applications.}},\n"
    "    booktitle = {Proceedings of the 16th ACM SIGKDD International "
    "Conference\n        on Knowledge Discovery and Data Mining},\n"
    "    series = {KDD '10},\n"
    "    year = {2010}\n"
    "  }\n\n"
    "The output is saved in a three-column matrix, where each row indicates an "
    "edge.  The first column corresponds to the lesser index of the edge; the "
    "second column corresponds to the greater index of the edge; and the third "
    "column corresponds to the distance between the two points.");

PARAM_STRING_REQ("input_file", "Data input file.", "i");
PARAM_STRING("output_file", "Data output file.  Stored as an edge list.", "o",
    "emst_output.csv");
PARAM_FLAG("naive", "Compute the MST using O(n^2) naive algorithm.", "n");
PARAM_INT("leaf_size", "Leaf size in the kd-tree.  One-element leaves give the "
    "empirically best performance, but at the cost of greater memory "
    "requirements.", "l", 1);

using namespace mlpack;
using namespace mlpack::emst;
using namespace mlpack::tree;

int main(int argc, char* argv[])
{
  CLI::ParseCommandLine(argc, argv);

  ///////////////// READ IN DATA //////////////////////////////////
  std::string dataFilename = CLI::GetParam<std::string>("input_file");

  Log::Info << "Reading in data.\n";

  arma::mat dataPoints;
  data::Load(dataFilename.c_str(), dataPoints, true);

  // Do naive.
  if (CLI::GetParam<bool>("naive"))
  {
    Log::Info << "Running naive algorithm.\n";

    DualTreeBoruvka<> naive(dataPoints, true);

    arma::mat naiveResults;
    naive.ComputeMST(naiveResults);

    std::string outputFilename = CLI::GetParam<std::string>("output_file");

    data::Save(outputFilename.c_str(), naiveResults, true);
  }
  else
  {
    Log::Info << "Data read, building tree.\n";

    /////////////// Initialize DTB //////////////////////
    if (CLI::GetParam<int>("leaf_size") <= 0)
    {
      Log::Fatal << "Invalid leaf size (" << CLI::GetParam<int>("leaf_size")
          << ")!  Must be greater than or equal to 1." << std::endl;
    }

    size_t leafSize = CLI::GetParam<int>("leaf_size");

    DualTreeBoruvka<> dtb(dataPoints, false, leafSize);

    Log::Info << "Tree built, running algorithm.\n";

    ////////////// Run DTB /////////////////////
    arma::mat results;

    dtb.ComputeMST(results);

    //////////////// Output the Results ////////////////
    std::string outputFilename = CLI::GetParam<std::string>("output_file");

    data::Save(outputFilename.c_str(), results, true);
  }

  return 0;
}
