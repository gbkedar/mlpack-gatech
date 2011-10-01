/***
 * @file allknn_evaluate_metric.h
 * @author Ryan Curtin
 *
 * When we have a weighting vector generated with allknn_learn_metric, we can
 * evaluate it with this utility.
 */

#ifndef __MLPACK_CONTRIB_RCURTIN_ALLKNN_EVALUATE_METRIC_H
#define __MLPACK_CONTRIB_RCURTIN_ALLKNN_EVALUATE_METRIC_H

#include <fastlib/fastlib.h>
#include <mlpack/neighbor_search/neighbor_search.h>

#include "allknn_metric_utils.h"

// Define options
PARAM_STRING_REQ("reference_file", "allknn_evaluate_metric", "Input CSV file.");
PARAM_STRING_REQ("query_file", "allknn_evaluate_metric", "Query CSV file.");
PARAM_STRING_REQ("weights_file", "allknn_evaluate_metric", "CSV file containing"
    " weights to evaluate.");
PARAM_STRING_REQ("output_file", "allknn_evaluate_metric", "Output correct "
    "counts file.");

const fx_entry_doc allnn_timit_main[] = {
  {"reference_file", FX_REQUIRED, FX_STR, NULL, "Input CSV file."},
  {"query_file", FX_REQUIRED, FX_STR, NULL, "Query CSV file."},
  {"weights_file", FX_REQUIRED, FX_STR, NULL,
      "Input weights CSV file."},
  {"k", FX_REQUIRED, FX_INT, NULL, "k-nearest-neighbors value."},
  {"output_file", FX_REQUIRED, FX_STR, NULL, "Output correct counts file."},
  FX_ENTRY_DOC_DONE
};

const fx_module_doc allnn_timit_doc = {
  allnn_timit_main, NULL, "AllkNN Learned Metric Evaluation"
};

#endif
