#ifndef RATCLI_H
#define RATCLI_H

#include "operator.h"

class Ratio: public Operator {

 public:
  
  double NaiveCompute(std::map<size_t, size_t> &constant_dataset_indices) {

    double numerator_result = 
      operators_[0]->NaiveCompute(constant_dataset_indices);
    double denominator_result = 
      operators_[1]->NaiveCompute(constant_dataset_indices);
    double result = numerator_result / denominator_result;
    
    return PostProcess_(constant_dataset_indices, result);
  }

  double MonteCarloCompute
  (std::vector<Strata> &list_of_strata,
   std::map<size_t, size_t> &constant_dataset_indices,
   double relative_error, double probability) {

    double numerator_result = 
      operators_[0]->MonteCarloCompute
      (list_of_strata, constant_dataset_indices, relative_error, probability);
    double denominator_result = 
      operators_[1]->MonteCarloCompute
      (list_of_strata, constant_dataset_indices, relative_error, probability);
    double result = numerator_result / denominator_result;

    return PostProcess_(constant_dataset_indices, result);
  }

};

#endif
