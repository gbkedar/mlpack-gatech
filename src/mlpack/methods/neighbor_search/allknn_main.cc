/**
 * @file main.cc
 *
 * Implementation of the AllkNN executable.  Allows some number of standard
 * options.
 *
 * @author Ryan Curtin
 */
#include <mlpack/core.h>
#include "neighbor_search.h"

#include <string>
#include <fstream>
#include <iostream>

#include <armadillo>

using namespace std;
using namespace mlpack;
using namespace mlpack::neighbor;

// Information about the program itself.
PROGRAM_INFO("All K-Nearest-Neighbors",
    "This program will calculate the all k-nearest-neighbors of a set of "
    "points. You may specify a separate set of reference points and query "
    "points, or just a reference set which will be used as both the reference "
    "and query set."
    "\n\n"
    "For example, the following will calculate the 5 nearest neighbors of each"
    "point in 'input.csv' and store the results in 'output.csv':"
    "\n\n"
    "$ allknn --neighbor_search/k=5 --reference_file=input.csv\n"
    "  --output_file=output.csv", "neighbor_search");

// Define our input parameters that this program will take.
PARAM_STRING_REQ("reference_file", "CSV file containing the reference dataset.",
    "");
PARAM_STRING("query_file", "CSV file containing query points (optional).",
    "", "");
PARAM_STRING_REQ("output_file", "File to output CSV-formatted results into.",
    "");

int main(int argc, char *argv[]) {
  // Give IO the command line parameters the user passed in.
  IO::ParseCommandLine(argc, argv);

  string reference_file = IO::GetParam<string>("reference_file");
  string output_file = IO::GetParam<string>("output_file");

  arma::mat reference_data;

  arma::Mat<size_t> neighbors;
  arma::mat distances;

  if (data::Load(reference_file.c_str(), reference_data) == false)
    IO::Fatal << "Reference file " << reference_file << " not found." << endl;

  IO::Info << "Loaded reference data from " << reference_file << endl;

  // Sanity check on k value: must be greater than 0, must be less than the
  // number of reference points.
  size_t k = IO::GetParam<int>("neighbor_search/k");
  if ((k <= 0) || (k >= reference_data.n_cols)) {
    IO::Fatal << "Invalid k: " << k << "; must be greater than 0 and less ";
    IO::Fatal << "than the number of reference points (";
    IO::Fatal << reference_data.n_cols << ")." << endl;
  }

  // Sanity check on leaf size.
  if (IO::GetParam<int>("tree/leaf_size") <= 0) {
    IO::Fatal << "Invalid leaf size: " << IO::GetParam<int>("allknn/leaf_size")
        << endl;
  }

  AllkNN* allknn = NULL;

  if (IO::GetParam<string>("query_file") != "") {
    string query_file = IO::GetParam<string>("query_file");
    arma::mat query_data;

    if (data::Load(query_file.c_str(), query_data) == false)
      IO::Fatal << "Query file " << query_file << " not found" << endl;

    IO::Info << "Query data loaded from " << query_file << endl;

    IO::Info << "Building query and reference trees..." << endl;
    allknn = new AllkNN(query_data, reference_data);

  } else {
    IO::Info << "Building reference tree..." << endl;
    allknn = new AllkNN(reference_data);
  }

  IO::Info << "Tree(s) built." << endl;

  IO::Info << "Computing " << k << " nearest neighbors..." << endl;
  allknn->ComputeNeighbors(neighbors, distances);

  IO::Info << "Neighbors computed." << endl;
  IO::Info << "Exporting results..." << endl;

  // Should be using data::Save or a related function instead of being written
  // by hand.
  try {
    ofstream out(output_file.c_str());

    for (size_t col = 0; col < neighbors.n_cols; col++) {
      out << col << ", ";
      for (size_t j = 0; j < (k - 1) /* last is special case */; j++) {
        out << neighbors(j, col) << ", " << distances(j, col) << ", ";
      }
      out << neighbors((k - 1), col) << ", " << distances((k - 1), col) << endl;
    }

    out.close();
  } catch(exception& e) {
    IO::Fatal << "Error while opening " << output_file << ": " << e.what()
        << endl;
  }

  delete allknn;
}