/**
 * @file dt_utils.cpp
 * @author Parikshit Ram (pram@cc.gatech.edu)
 *
 * This file implements functions to perform different tasks with the Density
 * Tree class.
 */
#include "dt_utils.hpp"

using namespace mlpack;
using namespace det;

void mlpack::det::PrintLeafMembership(DTree* dtree,
                                      const arma::mat& data,
                                      const arma::Mat<size_t>& labels,
                                      const size_t numClasses,
                                      const std::string leafClassMembershipFile)
{
  // Tag the leaves with numbers.
  int numLeaves = dtree->TagTree();

  arma::Mat<size_t> table(numLeaves, numClasses);
  table.zeros();

  for (size_t i = 0; i < data.n_cols; i++)
  {
    const arma::vec testPoint = data.unsafe_col(i);
    const int leafTag = dtree->FindBucket(testPoint);
    const size_t label = labels[i];
    table(leafTag, label) += 1;
  }

  if (leafClassMembershipFile == "")
  {
    Log::Info << "Leaf membership; row represents leaf id, column represents "
        << "class id; value represents number of points in leaf in class."
        << std::endl << table;
  }
  else
  {
    // Create a stream for the file.
    std::ofstream outfile(leafClassMembershipFile.c_str());
    if (outfile.good())
    {
      outfile << table;
      Log::Info << "Leaf membership printed to '" << leafClassMembershipFile
          << "'." << std::endl;
    }
    else
    {
      Log::Warn << "Can't open '" << leafClassMembershipFile << "' to write "
          << "leaf membership to." << std::endl;
    }
    outfile.close();
  }

  return;
}


void mlpack::det::PrintVariableImportance(const DTree* dtree,
                                          const std::string viFile)
{
  arma::vec imps;
  dtree->ComputeVariableImportance(imps);

  double max = 0.0;
  for (size_t i = 0; i < imps.n_elem; ++i)
    if (imps[i] > max)
      max = imps[i];

  Log::Info << "Maximum variable importance: " << max << "." << std::endl;

  if (viFile == "")
  {
    Log::Info << "Variable importance: " << std::endl << imps.t() << std::endl;
  }
  else
  {
    std::ofstream outfile(viFile.c_str());
    if (outfile.good())
    {
      outfile << imps;
      Log::Info << "Variable importance printed to '" << viFile << "'."
          << std::endl;
    }
    else
    {
      Log::Warn << "Can't open '" << viFile << "' to write variable importance "
          << "to." << std::endl;
    }
    outfile.close();
  }
}


// This function trains the optimal decision tree using the given number of
// folds.
DTree* mlpack::det::Trainer(arma::mat& dataset,
                            const size_t folds,
                            const bool useVolumeReg,
                            const size_t maxLeafSize,
                            const size_t minLeafSize,
                            const std::string unprunedTreeOutput)
{
  // Initialize the tree.
  DTree* dtree = new DTree(dataset);

  // Prepare to grow the tree...
  arma::Col<size_t> oldFromNew(dataset.n_cols);
  for (size_t i = 0; i < oldFromNew.n_elem; i++)
    oldFromNew[i] = i;

  // Save the dataset since it would be modified while growing the tree.
  arma::mat newDataset(dataset);

  // Growing the tree
  double oldAlpha = 0.0;
  double alpha = dtree->Grow(newDataset, oldFromNew, useVolumeReg, maxLeafSize,
      minLeafSize);

  Log::Info << dtree->SubtreeLeaves() << " leaf nodes in the tree using full "
      << "dataset; minimum alpha: " << alpha << "." << std::endl;

  // Compute densities for the training points in the full tree, if we were
  // asked for this.
  if (unprunedTreeOutput != "")
  {
    std::ofstream outfile(unprunedTreeOutput.c_str());
    if (outfile.good())
    {
      for (size_t i = 0; i < dataset.n_cols; ++i)
      {
        arma::vec testPoint = dataset.unsafe_col(i);
        outfile << dtree->ComputeValue(testPoint) << std::endl;
      }
    }
    else
    {
      Log::Warn << "Can't open '" << unprunedTreeOutput << "' to write computed"
          << " densities to." << std::endl;
    }

    outfile.close();
  }

  // Sequentially prune and save the alpha values and the values of c_t^2 * r_t.
  std::vector<std::pair<double, double> > prunedSequence;
  while (dtree->SubtreeLeaves() > 1)
  {
    std::pair<double, double> treeSeq(oldAlpha,
        dtree->SubtreeLeavesLogNegError());
    prunedSequence.push_back(treeSeq);
    oldAlpha = alpha;
    alpha = dtree->PruneAndUpdate(oldAlpha, dataset.n_cols, useVolumeReg);

    // Some sanity checks.
    Log::Assert((alpha < std::numeric_limits<double>::max()) ||
        (dtree->SubtreeLeaves() == 1));
    Log::Assert(alpha > oldAlpha);
    Log::Assert(dtree->SubtreeLeavesLogNegError() < treeSeq.second);
  }

  std::pair<double, double> treeSeq(oldAlpha,
      dtree->SubtreeLeavesLogNegError());
  prunedSequence.push_back(treeSeq);

  Log::Info << prunedSequence.size() << " trees in the sequence; maximum alpha:"
      << " " << oldAlpha << "." << std::endl;

  delete dtree;

  arma::mat cvData(dataset);
  size_t testSize = dataset.n_cols / folds;

  // Go through each fold.
  for (size_t fold = 0; fold < folds; fold++)
  {
    // Break up data into train and test sets.
    size_t start = fold * testSize;
    size_t end = std::min((fold + 1) * testSize, (size_t) cvData.n_cols);

    arma::mat test = cvData.cols(start, end - 1);
    arma::mat train(cvData.n_rows, cvData.n_cols - test.n_cols);

    if (start == 0 && end < cvData.n_cols)
    {
      train.cols(0, train.n_cols - 1) = cvData.cols(end, cvData.n_cols - 1);
    }
    else if (start > 0 && end == cvData.n_cols)
    {
      train.cols(0, train.n_cols - 1) = cvData.cols(0, start - 1);
    }
    else
    {
      train.cols(0, start - 1) = cvData.cols(0, start - 1);
      train.cols(start, train.n_cols - 1) = cvData.cols(end, cvData.n_cols - 1);
    }

    // Initialize the tree.
    DTree* cvDTree = new DTree(train);

    // Getting ready to grow the tree...
    arma::Col<size_t> cvOldFromNew(train.n_cols);
    for (size_t i = 0; i < cvOldFromNew.n_elem; i++)
      cvOldFromNew[i] = i;

    // Grow the tree.
    oldAlpha = 0.0;
    alpha = cvDTree->Grow(train, cvOldFromNew, useVolumeReg, maxLeafSize,
        minLeafSize);

    // Sequentially prune with all the values of available alphas and adding
    // values for test values.
    std::vector<std::pair<double, double> >::iterator it;
    for (it = prunedSequence.begin(); it < prunedSequence.end() - 2; ++it)
    {
      // Compute test values for this state of the tree.
      double cvVal = 0.0;
      for (size_t i = 0; i < test.n_cols; i++)
      {
        arma::vec testPoint = test.unsafe_col(i);
        cvVal += cvDTree->ComputeValue(testPoint);
      }

      // Update the cv error value by mapping out of log-space then back into
      // it, using long doubles.
      long double notLogVal = -std::exp((long double) it->second) -
          2.0 * cvVal / (double) dataset.n_cols;
      it->second = (double) std::log(-notLogVal);

      // Determine the new alpha value and prune accordingly.
      oldAlpha = sqrt(((it + 1)->first) * ((it + 2)->first));
      alpha = cvDTree->PruneAndUpdate(oldAlpha, train.n_cols, useVolumeReg);
    }

    // Compute test values for this state of the tree.
    double cvVal = 0.0;
    for (size_t i = 0; i < test.n_cols; ++i)
    {
      arma::vec testPoint = test.unsafe_col(i);
      cvVal += cvDTree->ComputeValue(testPoint);
    }

    // Update the cv error value.
    long double notLogVal = -std::exp((long double) it->second) -
        2.0 * cvVal / (double) dataset.n_cols;
    it->second -= (double) std::log(-notLogVal);

    test.reset();
    delete cvDTree;
  }

  double optimalAlpha = -1.0;
  double cvBestError = std::numeric_limits<double>::max();
  std::vector<std::pair<double, double> >::iterator it;

  for (it = prunedSequence.begin(); it < prunedSequence.end() -1; ++it)
  {
    if (it->second < cvBestError)
    {
      cvBestError = it->second;
      optimalAlpha = it->first;
    }
  }

  Log::Info << "Optimal alpha: " << optimalAlpha << "." << std::endl;

  // Initialize the tree.
  DTree* dtreeOpt = new DTree(dataset);

  // Getting ready to grow the tree...
  for (size_t i = 0; i < oldFromNew.n_elem; i++)
    oldFromNew[i] = i;

  // Save the dataset since it would be modified while growing the tree.
  newDataset = dataset;

  // Grow the tree.
  oldAlpha = 0.0;
  alpha = dtreeOpt->Grow(newDataset, oldFromNew, useVolumeReg, maxLeafSize,
      minLeafSize);

  // Prune with optimal alpha.
  while ((oldAlpha > optimalAlpha) && (dtreeOpt->SubtreeLeaves() > 1))
  {
    oldAlpha = alpha;
    alpha = dtreeOpt->PruneAndUpdate(oldAlpha, newDataset.n_cols, useVolumeReg);

    // Some sanity checks.
    Log::Assert((alpha < std::numeric_limits<double>::max()) ||
        (dtreeOpt->SubtreeLeaves() == 1));
    Log::Assert(alpha < oldAlpha);
  }

  Log::Info << dtreeOpt->SubtreeLeaves() << " leaf nodes in the optimally "
      << "pruned tree; optimal alpha: " << oldAlpha << "." << std::endl;

  return dtreeOpt;
}
