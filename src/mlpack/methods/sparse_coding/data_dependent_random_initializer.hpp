/**
 * @file data_dependent_random_initializer.hpp
 * @author Nishant Mehta
 *
 * A sensible heuristic for initializing dictionaries for sparse coding.
 */
#ifndef __MLPACK_METHODS_SPARSE_CODING_DATA_DEPENDENT_RANDOM_INITIALIZER_HPP
#define __MLPACK_METHODS_SPARSE_CODING_DATA_DEPENDENT_RANDOM_INITIALIZER_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace sparse_coding {

/**
 * A data-dependent random dictionary initializer for SparseCoding.  This
 * creates random dictionary atoms by adding three random observations from the
 * data together, and then normalizing the atom.
 */
class DataDependentRandomInitializer
{
 public:
  /**
   * Initialize the dictionary by adding together three random observations from
   * the data, and then normalizing the atom.  This implementation is simple
   * enough to be included with the definition.
   *
   * @param data Dataset to initialize the dictionary with.
   * @param atoms Number of atoms in dictionary.
   * @param dictionary Dictionary to initialize.
   */
  static void Initialize(const arma::mat& data,
                         const size_t atoms,
                         arma::mat& dictionary)
  {
    // Set the size of the dictionary.
    dictionary.set_size(data.n_rows, atoms);

    // Create each atom.
    for (size_t i = 0; i < atoms; ++i)
    {
      // Add three atoms together.
      dictionary.col(i) = (data.col(math::RandInt(data.n_cols)) +
          data.col(math::RandInt(data.n_cols)) +
          data.col(math::RandInt(data.n_cols)));

      // Now normalize the atom.
      dictionary.col(i) /= norm(dictionary.col(i), 2);
    }
  }
};

}; // namespace sparse_coding
}; // namespace mlpack

#endif
