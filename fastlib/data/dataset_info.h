/**
 * @file dataset_info.h
 *
 * Declaration of DatasetInfo class, which is a helper class for Dataset.
 * 
 * @bug These routines fail when trying to read files linewise that use the Mac
 * eol '\r'.  Both Windows and Unix eol ("\r\n" and '\n') work.  Use the
 * programs 'dos2unix' or 'tr' to convert the '\r's to '\n's.
 *
 */

#ifndef DATA_DATASET_INFO_H
#define DATA_DATASET_INFO_H

#include <armadillo>
#include <string>
#include <vector>

#include "../file/textfile.h"

#include "dataset_feature.h"

class TextLineReader;
class TextWriter;

/**
 * Information describing a dataset and its features.
 */
class DatasetInfo {
 private:
  std::string name_;
  std::vector<DatasetFeature> features_;

 public:
  /** Gets a mutable list of all features. */
  std::vector<DatasetFeature>& features() {
    return features_;
  }

  /** Gets information about a particular feature. */
  const DatasetFeature& feature(size_t attrib_num) const {
    return features_[attrib_num];
  }

  /** Gets the number of features. */
  size_t n_features() const {
    return features_.size();
  }

  /** Gets the title of the data set. */
  const std::string& name() const {
    return name_;
  }

  /** Sets the title of the data set. */
  void set_name(const std::string& name_in) {
    name_ = name_in;
  }

  /**
   * Checks if all parameters are continuous.
   */
  bool is_all_continuous() const;

  /**
   * Initialize an all-continuous dataset;
   *
   * @param n_features the number of continuous features
   * @param name_in the dataset title
   */
  void InitContinuous(size_t n_features, 
                      const std::string& name_in = "dataset");

  /**
   * Initialize a custom dataset.
   *
   * This assumes you will eventually use the features() to add features.
   *
   * @param name_in the dataset title
   */
  void Init(const std::string& name_in = "dataset");

  /**
   * Copy constructor, written manually to avoid use of OBJECT_TRAVERSAL functions.
   * TODO: Make this Init() crap go away.
   */
  void InitCopy(const DatasetInfo &info) {
    name_ = info.name();
    features_ = info.features_;
  }

  /**
   * Writes the header for an ARFF file.
   */
  void WriteArffHeader(TextWriter& writer) const;

  /**
   * Writes header for CSV file.
   *
   * @param sep the value separator (use ",\t" for CSV)
   * @param writer the text writer to write the header line to
   */
  void WriteCsvHeader(const char *sep, TextWriter& writer) const;

  /**
   * Writes the contents of a matrix to a file.
   *
   * @param matrix the matrix
   * @param sep the separator (use ",\t" for CSV)
   * @param writer the writer to write to
   */
  void WriteMatrix(const arma::mat& matrix, const char *sep,
      TextWriter& writer) const;

  /**
   * Initialize explicitly from an ARFF file.
   *
   * ARFF LIMITATIONS: Values cannot have spaces or commas, even with quotes;
   * 'string' data type not supported (nominal is supported).
   *
   * You might just use InitFromFile, which will guess the type for you.
   *
   * This will read only the header information and leave the reader at the
   * first line of data.
   */
  bool InitFromArff(TextLineReader& reader,
      const std::string& filename = "dataset");

  /**
   * Initialize from a CSV-like file with numbers only, inferring
   * automatically that if the first row has non-numeric characters, it is
   * a header.
   *
   * InitFromFile will automatically detect this.
   */
  bool InitFromCsv(TextLineReader& reader,
      const std::string& filename = "dataset");

  /**
   * Initializes the header from a file, either CSV or ARFF.
   *
   * All header lines will be gobbled, so the reader's position will be
   * left at the first line of actual data.
   * You can then read the data with matrix.
   */
  bool InitFromFile(TextLineReader& reader,
      const std::string& filename = "dataset");

  /**
   * Populates a matrix from a file, given the internal data model.
   *
   * ARFF LIMITATIONS: Values cannot have spaces or commas, even with quotes;
   * 'string' data type not supported (nominal is supported).
   *
   * @param reader the reader to get lines from
   * @param matrix the matrix to store text into
   */
  bool ReadMatrix(TextLineReader& reader, arma::mat& matrix) const;

  /**
   * Reads a single vector.
   *
   * @param reader the line reader being used
   * @param col a mat::col_iterator referencing the beginning of the
   *        column that we are reading into
   * @param is_done set to true if we have finished reading the file
   *        successfully -- the value of is_done is undefined if the
   *        function returns failure!
   * @return whether reading the line was successful
   */
  bool ReadPoint(TextLineReader& reader, arma::mat::col_iterator col,
      bool& is_done) const;

 private:
  size_t SkipSpace_(std::string& s);
  char *SkipNonspace_(char *s);
  void SkipBlanks_(TextLineReader& reader);
};

#endif
