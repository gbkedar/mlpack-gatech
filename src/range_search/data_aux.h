#ifndef DATA_AUX_H
#define DATA_AUX_H

#include "fastlib/fastlib.h"

namespace data_aux {

  /**
   * Loads a matrix from a file.
   *
   * This supports any type the Dataset class supports with the
   * InitFromFile function: CSV and ARFF.
   *
   * @code
   * Matrix A;
   * data::Load("foo.csv", &A);
   * @endcode
   *
   * @param fname the file name to load
   * @param matrix a pointer to an uninitialized matrix to load
   */
  template<typename T>
  static bool Load(const char *fname, GenMatrix<T> *matrix) {
    Matrix tmp_matrix;
    bool result = data::Load(fname, &tmp_matrix);

    // Allocate the matrix that is to be returned and copy all
    // entries.
    matrix->StaticInit(tmp_matrix.n_rows(), tmp_matrix.n_cols());
    for(size_t c = 0; c < tmp_matrix.n_cols(); c++) {
      for(size_t r = 0; r < tmp_matrix.n_rows(); r++) {
	matrix->set(r, c, static_cast<T>(tmp_matrix.get(r, c)));
      }
    }

    return result;
  }

  /**
   * Loads a matrix from a file.
   *
   * This supports any type the Dataset class supports with the
   * InitFromFile function: CSV and ARFF.
   *
   * @code
   * Matrix A;
   * data::Load("foo.csv", &A);
   * @endcode
   *
   * @param fname the file name to load
   * @param matrix a pointer to an uninitialized matrix to load
   */
  template<typename T>
  static bool LoadTranspose(const char *fname, GenMatrix<T> *matrix) {
    Matrix tmp_matrix;
    bool result = data::Load(fname, &tmp_matrix);

    // Allocate the matrix that is to be returned and copy all
    // entries.
    matrix->StaticInit(tmp_matrix.n_cols(), tmp_matrix.n_rows());
    for(size_t c = 0; c < tmp_matrix.n_cols(); c++) {
      for(size_t r = 0; r < tmp_matrix.n_rows(); r++) {
	matrix->set(c, r, static_cast<T>(tmp_matrix.get(r, c)));
      }
    }

    return result;
  }

};

#endif
