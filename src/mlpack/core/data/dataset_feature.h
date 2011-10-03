/**
 * @file dataset_feature.h
 *
 * The DatasetFeature class, used by the Dataset class.
 *
 * @bug These routines fail when trying to read files linewise that use the Mac
 * eol '\r'.  Both Windows and Unix eol ("\r\n" and '\n') work.  Use the
 * programs 'dos2unix' or 'tr' to convert the '\r's to '\n's.
 *
 */

#ifndef DATA_DATASET_FEATURE_H
#define DATA_DATASET_FEATURE_H

#include <string>
#include <vector>

#include "../io/io.h"
#include "../file/textfile.h"

/**
 * Metadata about a particular dataset feature (attribute).
 *
 * Supports nominal, continuous, and integer values.
 */
class DatasetFeature {
 public:
  /**
   * Supported feature types.
   */
  enum Type {
    CONTINUOUS, /** Real-valued data. */
    INTEGER,  /** Integer valued data. */
    NOMINAL  /** Discrete data, each of which has a "name". */
  };

 private:
  std::string name_; /** Name of the feature. */
  Type type_; /** Type of data this feature represents. */
  std::vector<std::string> value_names_; /** If nominal, the names of each numbered value. */
  static const double DBL_NAN; /** Redefinition for ease of use */

  /**
   * Initialization common to all features.
   *
   * @param name_in the name of the feature
   */
  void InitGeneral(const std::string& name_in) {
    name_ = name_in;
  }

 public:
  /**
   * Initialize to be a continuous feature.
   *
   * @param name_in the name of the feature
   */
  void InitContinuous(const std::string& name_in) {
    InitGeneral(name_in);
    type_ = CONTINUOUS;
  }

  /**
   * Initializes to an integer type.
   *
   * @param name_in the name of the feature
   */
  void InitInteger(const std::string& name_in) {
    InitGeneral(name_in);
    type_ = INTEGER;
  }

  /**
   * Initializes to a nominal type.
   *
   * The value_names list starts empty, so you need to add the name of
   * each feature to this.  (The dataset reading functions will do this
   * for you).
   *
   * @param name_in the name of the feature
   */
  void InitNominal(const std::string& name_in) {
    InitGeneral(name_in);
    type_ = NOMINAL;
  }

  /**
   * Creates a text version of the value based on the type.
   *
   * Continuous parameters are printed in floating point, and integers
   * are shown as integers.  For nominal, the value_name(int(value)) is
   * shown.  NaN (missing data) is always shown as '?'.
   *
   * @param value the value to format
   * @param result this will be initialized to the formatted text
   */
  void Format(double value, std::string& result) const;

  /**
   * Parses a string into the particular value.
   *
   * Integers and continuous are parsed using the normal functions.
   * For nominal, the entry
   *
   * If an invalid parse occurs, such as a mal-formatted number or
   * a nominal value not in the list, false will be returned.
   *
   * @param str the string to parse
   * @param d where to store the result
   */
  bool Parse(const std::string& str, double& d) const;

  /**
   * Gets what the feature is named.
   *
   * @return the name of the feature; for point, "Age" or "X Position"
   */
  const std::string& name() const {
    return name_;
  }

  /**
   * Identifies the type of feature.
   *
   * @return whether this is DatasetFeature::CONTINUOUS, INTEGER, or NOMINAL
   */
  Type type() const {
    return type_;
  }

  /**
   * Returns the name of a particular nominal value, given its index.
   *
   * The first nominal value is 0, the second is 1, etc.
   *
   * @param value the number of the value
   */
  const std::string& value_name(int value) const {
    mlpack::IO::Assert(type_ == NOMINAL);
    return value_names_[value];
  }

  /**
   * The number of nominal values.
   *
   * The values 0 to n_values() - 1 are valid.
   * This will return zero for CONTINUOUS and INTEGER types.
   *
   * @return the number of nominal values
   */
  size_t n_values() const {
    return value_names_.size();
  }

  /**
   * Gets the array of value names.
   *
   * Useful for creating a nominal feature yourself.
   *
   * @return a mutable array of value names
   */
  std::vector<std::string>& value_names() {
    return value_names_;
  }
};

#endif
