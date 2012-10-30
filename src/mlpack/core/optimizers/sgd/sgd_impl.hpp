/**
 * @file sgd_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of stochastic gradient descent.
 */
#ifndef __MLPACK_CORE_OPTIMIZERS_SGD_SGD_IMPL_HPP
#define __MLPACK_CORE_OPTIMIZERS_SGD_SGD_IMPL_HPP

// In case it hasn't been included yet.
#include "sgd.hpp"

namespace mlpack {
namespace optimization {

template<typename DecomposableFunctionType>
SGD<DecomposableFunctionType>::SGD(DecomposableFunctionType& function,
                                   const double stepSize,
                                   const size_t maxIterations,
                                   const double tolerance,
                                   const bool shuffle) :
    function(function),
    stepSize(stepSize),
    maxIterations(maxIterations),
    tolerance(tolerance),
    shuffle(shuffle)
{ /* Nothing to do. */ }

//! Optimize the function (minimize).
template<typename DecomposableFunctionType>
double SGD<DecomposableFunctionType>::Optimize(arma::mat& iterate)
{
  // Find the number of functions to use.
  const size_t numFunctions = function.NumFunctions();

  // This is used only if shuffle is true.
  arma::vec visitationOrder;
  if (shuffle)
    visitationOrder = arma::shuffle(arma::linspace(0, (numFunctions - 1),
        numFunctions));

  // To keep track of where we are and how things are going.
  size_t currentFunction = 0;
  double overallObjective = 0;
  double lastObjective = DBL_MAX;

  // Calculate the first objective function.
  for (size_t i = 0; i < numFunctions; ++i)
    overallObjective += function.Evaluate(iterate, i);

  // Now iterate!
  arma::mat gradient(iterate.n_rows, iterate.n_cols);
  for (size_t i = 1; i != maxIterations; ++i, ++currentFunction)
  {
    // Is this iteration the start of a sequence?
    if ((currentFunction % numFunctions) == 0)
    {
      // Output current objective function.
      Log::Info << "SGD: iteration " << i << ", objective " << overallObjective
          << "." << std::endl;

      if (overallObjective != overallObjective)
      {
        Log::Warn << "SGD: converged to " << overallObjective << "; terminating"
            << " with failure.  Try a smaller step size?" << std::endl;
        return overallObjective;
      }

      if (std::abs(lastObjective - overallObjective) < tolerance)
      {
        Log::Info << "SGD: minimized within tolerance " << tolerance << "; "
            << "terminating optimization." << std::endl;
        return overallObjective;
      }

      // Reset the counter variables.
      lastObjective = overallObjective;
      overallObjective = 0;
      currentFunction = 0;

      if (shuffle) // Determine order of visitation.
        visitationOrder = arma::shuffle(visitationOrder);
    }

    // Evaluate the gradient for this iteration.
    function.Gradient(iterate, currentFunction, gradient);

    // And update the iterate.
    iterate -= stepSize * gradient;

    // Now add that to the overall objective function.
    overallObjective += function.Evaluate(iterate, currentFunction);
  }

  Log::Info << "SGD: maximum iterations (" << maxIterations << ") reached; "
      << "terminating optimization." << std::endl;
  return overallObjective;
}

}; // namespace optimization
}; // namespace mlpack

#endif
