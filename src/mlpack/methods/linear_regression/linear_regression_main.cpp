/**
 * @file linear_regression_main.cpp
 * @author James Cline
 *
 * Main function for least-squares linear regression.
 */
#include <mlpack/core.hpp>
#include "linear_regression.hpp"

PROGRAM_INFO("Simple Linear Regression Prediction",
    "An implementation of simple linear regression using ordinary least "
    "squares. This solves the problem\n\n"
    "  y = X * b + e\n\n"
    "where X (--input_file) and y (the last row of --input_file, or "
    "--input_responses) are known and b is the desired variable.  The "
    "calculated b is saved to disk (--output_file).\n"
    "\n"
    "Optionally, the calculated value of b is used to predict the responses for"
    " another matrix X' (--test_file):\n\n"
    "   y' = X' * b\n\n"
    "and these predicted responses, y', are saved to a file "
    "(--output_predictions).");

PARAM_STRING("input_file", "File containing X (regressors).", "i", "");
PARAM_STRING("input_responses", "Optional file containing y (responses). If "
    "not given, the responses are assumed to be the last row of the input "
    "file.", "r", "");

PARAM_STRING("model_file", "File containing existing model (parameters).", "m",
    "");

PARAM_STRING("output_file", "File where parameters (b) will be saved.",
    "o", "parameters.csv");

PARAM_STRING("test_file", "File containing X' (test regressors).", "t", "");
PARAM_STRING("output_predictions", "If --test_file is specified, this file is "
    "where the predicted responses will be saved.", "p", "predictions.csv");

using namespace mlpack;
using namespace mlpack::regression;
using namespace arma;
using namespace std;

int main(int argc, char* argv[])
{
  // Handle parameters
  CLI::ParseCommandLine(argc, argv);

  const string modelName = CLI::GetParam<string>("model_file");
  const string outputFile = CLI::GetParam<string>("output_file");
  const string outputPredictions = CLI::GetParam<string>("output_predictions");
  const string responseName = CLI::GetParam<string>("input_responses");
  const string testName = CLI::GetParam<string>("test_file");
  const string trainName = CLI::GetParam<string>("input_file");

  mat regressors;
  mat responses;

  LinearRegression lr;

  bool computeModel;

  // We want to determine if an input file XOR model file were given.
  if (trainName.empty()) // The user specified no input file.
  {
    if (modelName.empty()) // The user specified no model file, error and exit.
      Log::Fatal << "You must specify either --input_file or --model_file."
          << endl;
    else // The model file was specified, no problems.
      computeModel = false;
  }
  // The user specified an input file but no model file, no problems.
  else if (modelName.empty())
    computeModel = true;

  // The user specified both an input file and model file.
  // This is ambiguous -- which model should we use? A generated one or given
  // one?  Report error and exit.
  else
  {
    Log::Fatal << "You must specify either --input_file or --model_file, not "
        << "both." << endl;
  }

  // If they specified a model file, we also need a test file or we
  // have nothing to do.
  if (!computeModel && testName.empty())
  {
    Log::Fatal << "When specifying --model_file, you must also specify "
        << "--test_file." << endl;
  }

  // An input file was given and we need to generate the model.
  if (computeModel)
  {
    Timer::Start("load_regressors");
    data::Load(trainName, regressors, true);
    Timer::Stop("load_regressors");

    // Are the responses in a separate file?
    if (responseName.empty())
    {
      // The initial predictors for y, Nx1.
      responses = trans(regressors.row(regressors.n_rows - 1));
      regressors.shed_row(regressors.n_rows - 1);
    }
    else
    {
      // The initial predictors for y, Nx1.
      Timer::Start("load_responses");
      data::Load(responseName, responses, true);
      Timer::Stop("load_responses");

      if (responses.n_rows == 1)
        responses = trans(responses); // Probably loaded backwards.

      if (responses.n_cols > 1)
        Log::Fatal << "The responses must have one column.\n";

      if (responses.n_rows != regressors.n_cols)
        Log::Fatal << "The responses must have the same number of rows as the "
            "training file.\n";
    }

    Timer::Start("regression");
    lr = LinearRegression(regressors, responses.unsafe_col(0));
    Timer::Stop("regression");

    // Save the parameters.
    data::Save(outputFile, lr.Parameters(), true);
  }

  // Did we want to predict, too?
  if (!testName.empty())
  {
    // A model file was passed in, so load it.
    if (!computeModel)
    {
      Timer::Start("load_model");
      lr = LinearRegression(modelName);
      Timer::Stop("load_model");
    }

    // Load the test file data.
    arma::mat points;
    Timer::Start("load_test_points");
    data::Load(testName, points, true);
    Timer::Stop("load_test_points");

    // Ensure that test file data has the right number of features.
    if ((lr.Parameters().n_elem - 1) != points.n_rows)
    {
      Log::Fatal << "The model was trained on " << lr.Parameters().n_elem - 1
          << "-dimensional data, but the test points in '" << testName
          << "' are " << points.n_rows << "-dimensional!" << endl;
    }

    // Perform the predictions using our model.
    arma::vec predictions;
    Timer::Start("prediction");
    lr.Predict(points, predictions);
    Timer::Stop("prediction");

    // Save predictions.
    predictions = arma::trans(predictions);
    data::Save(outputPredictions, predictions, true);
  }
}
