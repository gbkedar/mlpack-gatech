/**
 * @author Parikshit Ram (pram@cc.gatech.edu)
 * @file nbc_main.cc
 * 
 * This program test drives the Simple Naive Bayes Classifier
 * 
 * This classifier does parametric naive bayes classification
 * assuming that the features are sampled from a Gaussian
 * distribution.
 *
 * PARAMETERS TO BE INPUT:
 * 
 * --train 
 * This is the file that contains the training data
 *
 * --nbc/classes
 * This is the number of classes present in the training data
 *
 * --test
 * This file contains the data points which the trained
 * classifier would classify
 *
 * --output
 * This file will contain the classes to which the corresponding
 * data points in the testing data 
 * 
 */
#include "simple_nbc.h"
#include <fastlib/fx/io.h>

#include <armadillo>
#include <fastlib/data/dataset.h> 


/*const fx_entry_doc parm_nbc_main_entries[] = {
  {"train", FX_REQUIRED, FX_STR, NULL,
   " A file containing the training set\n"},
  {"test", FX_REQUIRED, FX_STR, NULL,
   " A file containing the test set\n"},
  {"output", FX_PARAM, FX_STR, NULL,
   " The file in which the output of the test would be "
   "written (defaults to 'output.csv')\n"},
  FX_ENTRY_DOC_DONE
};*/

PARAM_STRING_REQ("train", "A file containing the training set", "nbc");
PARAM_STRING_REQ("test", "A file containing the test set", "nbc");
PARAM_STRING("output", "The file in which the output of the test would\
 be written, defaults to 'output.csv')", "nbc", "output.csv");

/*const fx_submodule_doc parm_nbc_main_submodules[] = {
  {"nbc", &parm_nbc_doc,
   " Trains on a given set and number of classes and "
   "tests them on a given set\n"},
  FX_SUBMODULE_DOC_DONE
};*/

PARAM_MODULE("nbc", "Trains on a given set and number\
 of classes and tests them on a given set");

/*const fx_module_doc parm_nbc_main_doc = {
  parm_nbc_main_entries, parm_nbc_main_submodules,
  "This program test drives the Parametric Naive Bayes \n"
  "Classifier assuming that the features are sampled \n"
  "from a Gaussian distribution.\n"
};*/

PROGRAM_INFO("Parametric Naive Bayes", "This program test drives the\
 Parametric Naive Bayes Classifier assuming that the features are\
 sampled from a Gaussian distribution.", "nbc");

using namespace mlpack;

int main(int argc, char* argv[]) {

  IO::ParseCommandLine(argc, argv);

  ////// READING PARAMETERS AND LOADING DATA //////

  const char *training_data_filename = IO::GetParam<std::string>("nbc/train").c_str();
  arma::mat training_data;
  data::Load(training_data_filename, training_data);

  const char *testing_data_filename = IO::GetParam<std::string>("nbc/test").c_str();
  arma::mat testing_data;
  data::Load(testing_data_filename, testing_data);

  ////// SIMPLE NAIVE BAYES CLASSIFICATION ASSUMING THE DATA TO BE UNIFORMLY DISTRIBUTED //////
  
  ////// Timing the training of the Naive Bayes Classifier //////
  IO::StartTimer("nbc/training");

  ////// Create and train the classifier
  SimpleNaiveBayesClassifier nbc = SimpleNaiveBayesClassifier(training_data);

  ////// Stop training timer //////
  IO::StopTimer("nbc/training");

  ////// Timing the testing of the Naive Bayes Classifier //////
  ////// The variable that contains the result of the classification
  arma::vec results;

  IO::StartTimer("nbc/testing");

  ////// Calling the function that classifies the test data
  nbc.Classify(testing_data, results);

  ////// Stop testing timer //////
  IO::StopTimer("nbc/testing");

  ////// OUTPUT RESULTS //////
  std::string output_filename = IO::GetParam<std::string>("nbc/output");

  data::Save(output_filename.c_str(), results);

  return 1;
}
