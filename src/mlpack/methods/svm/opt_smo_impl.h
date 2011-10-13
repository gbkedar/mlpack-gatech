/**
 * @author Hua Ouyang
 *
 * @file opt_smo_impl.h
 *
 * This header file contains the implementation of the SMO class.
 *
 * The algorithms in the following papers are implemented:
 *
 * 1. SMO and Working set selecting using 1st order expansion
 * @ARTICLE{Platt_SMO,
 * author = "J. C. Platt",
 * title = "{Fast Training of Support Vector Machines using Sequential Minimal Optimization}",
 * booktitle = "{Advances in Kernel Methods - Support Vector Learning}",
 * year = 1999,
 * publisher = "MIT Press"
 * }
 *
 * 2. Shrinkng and Caching for SMO
 * @ARTICLE{Joachims_SVMLIGHT,
 * author = "T. Joachims",
 * title = "{Making large-Scale SVM Learning Practical}",
 * booktitle = "{Advances in Kernel Methods - Support Vector Learning}",
 * year = 1999,
 * publisher = "MIT Press"
 * }
 *
 * 3. Working set selecting using 2nd order expansion
 * @ARTICLE{Fan_JMLR,
 * author = "R. Fan, P. Chen, C. Lin",
 * title = "{Working Set Selection using Second Order Information for Training Support Vector Machines}",
 * journal = "{Jornal of Machine Learning Research}",
 * year = 2005
 * }
 *
 * @see svm.h
 */
#ifndef __MLPACK_METHODS_SVM_OPT_SMO_IMPL_H
#define __MLPACK_METHODS_SVM_OPT_SMO_IMPL_H

// In case it has not already been included.
#include "opt_smo.h"

namespace mlpack {
namespace svm {

template<typename TKernel>
void SMO<TKernel>::InitPara(int learner_typeid,
                            double Cp,
                            double CnEpsilon,
                            int hinge_sqhinge,
                            int wss,
                            size_t n_iter,
                            double accuracy) {
  // init parameters
  wss_ = wss;
  hinge_sqhinge_ = hinge_sqhinge;
  n_iter_ = (n_iter < MAX_NUM_ITER_SMO) ? n_iter: MAX_NUM_ITER_SMO;
  accuracy_ = accuracy;
  if (learner_typeid == 0) { // SVM_C
    if (hinge_sqhinge_ == 2) { // L2-SVM
      Cp_ = INFINITY;
      Cn_ = INFINITY;
      C_ = CnEpsilon;
      inv_two_C_ = 1 / (2 * C_);
    }
    else { // L1-SVM
      Cp_ = Cp;
      Cn_ = CnEpsilon;
    }
  }
  else if (learner_typeid == 1) { // SVM_R
    Cp_ = Cp;
    Cn_ = Cp_;
    epsilon_ = CnEpsilon;
  }
}

template<typename TKernel>
double SMO<TKernel>::CalcKernelValue_(size_t ii, size_t jj) {
  // the indices have been swaped in the shrinking processes
  size_t i = active_set_[ii]; // ii/jj: index in the new permuted set
  size_t j = active_set_[jj]; // i/j: index in the old set

  // for SVM_R where n_alpha_==2*n_data_
  if (learner_typeid_ == 1) {
    i = (i >= n_data_) ? (i - n_data_) : i;
    j = (j >= n_data_) ? (j - n_data_) : j;
  }

  // Check cache
  //if (i == i_cache_ && j == j_cache_) {
  //  return cached_kernel_value_;
  //}

  //v_i = datamatrix_samples_only_.GetColumnPtr(i);
  //v_j = datamatrix_samples_only_.GetColumnPtr(j);

  // Do Caching. Store the recently caculated kernel values.
  //i_cache_ = i;
  //j_cache_ = j;
  cached_kernel_value_ = kernel_.Eval(datamatrix_->col(i), datamatrix_->col(j), n_features_);

  if (hinge_sqhinge_ == 2) { // L2-SVM
    if (i == j) {
      cached_kernel_value_ = cached_kernel_value_ + inv_two_C_;
    }
  }

  return cached_kernel_value_;
}

/**
* Reconstruct inactive elements of G from G_bar and free variables
*
* @param: learner type id
*/
template<typename TKernel>
void SMO<TKernel>::ReconstructGradient_() {
  size_t i, j;
  if (n_active_ == n_alpha_)
    return;
  if (learner_typeid_ == 0) { // SVM_C
    for (i=n_active_; i<n_alpha_; i++) {
      grad_[i] = 1 - grad_bar_[i];
    }
  }
  else if (learner_typeid_ == 1) { // SVM_R
    for (i=n_active_; i<n_alpha_; i++) {
      j = i >= n_data_ ? (i-n_data_) : i;
      grad_[j] = grad_bar_[j] + (*datamatrix_)(datamatrix_->n_rows - 1, active_set_[j]) - epsilon_; // TODO
    }
  }

  for (i = 0; i < n_active_; i++) {
    if (alpha_status_[i] == SMO_ID_FREE) {
      for (j = n_active_; j < n_alpha_; j++) {
        grad_[j] = grad_[j] - y_[j] * alpha_[i] * y_[i] * CalcKernelValue_(i, j);
      }
    }
  }
}

/**
 * Test whether need to do shrinking for provided index and y_grad_max, y_grad_min
 *
 */
template<typename TKernel>
bool SMO<TKernel>::TestShrink_(size_t i, double y_grad_max, double y_grad_min) {
  if (IsUpperBounded(i)) { // alpha_[i] = C
    if (y_[i] == 1) {
      return (grad_[i] > y_grad_max);
    } else { // y_[i] == -1
      return (grad_[i] + y_grad_min > 0); // -grad_[i]<y_grad_min
    }
  } else if (IsLowerBounded(i)) {
    if (y_[i] == 1) {
      return (grad_[i] < y_grad_min);
    } else { // y_[i] == -1
      return (grad_[i] + y_grad_max < 0); // -grad_[i]>y_grad_max
    }
  }

  return false;
}

/**
 * Do Shrinking. Temporarily remove alphas (from the active set) that are
 * unlikely to be selected in the working set, since they have reached their
 * lower/upper bound.
 *
 */
template<typename TKernel>
void SMO<TKernel>::Shrinking_() {
  size_t t;

  // Find m(a) == y_grad_max(i\in I_up) and M(a) == y_grad_min(j\in I_down)
  double y_grad_max = -INFINITY;
  double y_grad_min =  INFINITY;
  for (t = 0; t < n_active_; t++) { // find argmax(y*grad), t\in I_up
    if (y_[t] == 1) {
      if (!IsUpperBounded(t)) // t\in I_up, y==1: y[t]alpha[t] < C
        if (grad_[t] > y_grad_max) { // y==1
          y_grad_max = grad_[t];
        }
    } else { // y[t] == -1
      if (!IsLowerBounded(t)) // t\in I_up, y==-1: y[t]alpha[t] < 0
        if (grad_[t] + y_grad_max < 0) { // y==-1... <=> -grad_[t] > y_grad_max
          y_grad_max = -grad_[t];
        }
    }
  }
  for (t = 0; t < n_active_; t++) { // find argmin(y*grad), t\in I_down
    if (y_[t] == 1) {
      if (!IsLowerBounded(t)) // t\in I_down, y==1: y[t]alpha[t] > 0
        if (grad_[t] < y_grad_min) { // y==1
          y_grad_min = grad_[t];
        }
    }
    else { // y[t] == -1
      if (!IsUpperBounded(t)) // t\in I_down, y==-1: y[t]alpha[t] > -C
        if (grad_[t] + y_grad_min > 0) { // y==-1...<=>  -grad_[t] < y_grad_min
          y_grad_min = -grad_[t];
        }
    }
  }

  // Find the alpha to be shrunk
  for (t = 0; t < n_active_; t++) {
    // Shrinking: put inactive alphas behind the active set
    if (TestShrink_(t, y_grad_max, y_grad_min)) {
      n_active_--;
      while (n_active_ > t) {
        if (!TestShrink_(n_active_, y_grad_max, y_grad_min)) {
          swap(active_set_[t], active_set_[n_active_]);
          swap(alpha_[t], alpha_[n_active_]);
          swap(alpha_status_[t], alpha_status_[n_active_]);
          swap(y_[t], y_[n_active_]);
          swap(grad_[t], grad_[n_active_]);
          swap(grad_bar_[t], grad_bar_[n_active_]);
          break;
        }

        n_active_--;
      }
    }
  }

  double gap = y_grad_max - y_grad_min;
  //printf("%d: gap:%f, n_active:%d\n", ct_iter_, gap, n_active_);
  // do unshrinking for the first time when y_grad_max - y_grad_min <= SMO_UNSHRINKING_FACTOR * accuracy_
  if (reconstructed_ == false && gap <= SMO_UNSHRINKING_FACTOR * accuracy_) {
    // Unshrinking: put shrinked alphas back to active set
    // 1.recover gradient
    ReconstructGradient_();
    // 2.recover active status
    for (t = n_alpha_ - 1; t > n_active_; t--) {
      if (!TestShrink_(t, y_grad_max, y_grad_min)) {
        while (n_active_ < t) {
          if (TestShrink_(n_active_, y_grad_max, y_grad_min)) {
            swap(active_set_[t], active_set_[n_active_]);
            swap(alpha_[t], alpha_[n_active_]);
            swap(alpha_status_[t], alpha_status_[n_active_]);
            swap(y_[t], y_[n_active_]);
            swap(grad_[t], grad_[n_active_]);
            swap(grad_bar_[t], grad_bar_[n_active_]);
            break;
          }

          n_active_++;
        }
        n_active_++;
      }
    }
    reconstructed_ = true; // indicator: unshrinking has been carried out in this round
  }
}

/**
 * Initialization according to different SVM learner types
 *
 * @param: learner type id
 */
template<typename TKernel>
void SMO<TKernel>::LearnersInit_(int learner_typeid) {
  size_t i;
  learner_typeid_ = learner_typeid;

  if (learner_typeid_ == 0) { // SVM_C
    n_alpha_ = n_data_;

    alpha_.set_size(n_alpha_);
    alpha_.zeros();

    // initialize gradient
    grad_.set_size(n_alpha_);
    grad_.fill(1.0);

    y_.resize(n_alpha_,0);
    for (i = 0; i < n_alpha_; i++) {
      y_[i] = (*datamatrix_)(datamatrix_->n_rows-1, i) > 0 ? 1 : -1;
    }
  } else if (learner_typeid_ == 1) { // SVM_R
    n_alpha_ = 2 * n_data_;

    alpha_.set_size(2 * n_alpha_); // TODO
    alpha_.zeros();

    // initialize gradient
    grad_.set_size(n_alpha_);
    y_.resize(n_alpha_,0);
    for (i = 0; i < n_data_; i++) {
      y_[i] = 1; // -> alpha_i
      y_[i + n_data_] = -1; // -> alpha_i^*
      grad_[i] = epsilon_ - (*datamatrix_)(datamatrix_->n_rows-1, i);
      grad_[i + n_data_] = epsilon_ + (*datamatrix_)(datamatrix_->n_rows-1, i);
    }
  } else if (learner_typeid_ == 2) { // SVM_DE
    // TODO
  }
}

/**
* SMO training for 2-classes
*
* @param: input 2-classes data matrix with labels (1,-1) in the last row
*/
template<typename TKernel>
void SMO<TKernel>::Train(int learner_typeid, arma::mat* dataset_in) {
  size_t i,j;
  // Load data, no deep copy
  datamatrix_ = dataset_in;
  n_data_ = dataset_in->n_cols;
  n_features_ = dataset_in->n_rows - 1; // excluding the last row for labels
  //datamatrix_samples_only_.Alias(dataset_in->ptr(), n_features_, n_data_);

  // Learners initialization
  LearnersInit_(learner_typeid);

  // General learner-independent initializations
  //budget_ = min(budget_, (int) n_data_);
  budget_ = (int) n_data_;
  bias_ = 0.0;
  n_sv_ = 0;
  reconstructed_ = false;
  i_cache_ = -1; j_cache_ = -1;
  cached_kernel_value_ = INFINITY;

  n_active_ = n_alpha_;
  active_set_.resize(n_alpha_,0);
  for (i = 0; i < n_alpha_; i++) {
    active_set_[i] = i;
  }

  alpha_status_.set_size(n_alpha_);
  for (i = 0; i < n_alpha_; i++)
    UpdateAlphaStatus_(i);

  // initialize gradient (already set to init values)
  /*
  for (i=0; i<n_alpha_; i++) {
    for(j=0; j<n_alpha_; j++) {
      if (!IsLowerbounded(j)) { // alpha_j >0
        grad_[i] = grad_[i] - y_[i] * y_[j] * alpha_[j] * CalcKernelValue_(i,j);
      }
    }
  }
  */

  // initialize gradient_bar
  grad_bar_.set_size(n_alpha_);
  grad_bar_.zeros();

  do_shrinking_ = CLI::GetParam<bool>("svm/shrink");
  ct_shrinking_ = std::min(n_data_, SMO_NUM_FOR_SHRINKING);
  if (do_shrinking_) {
    for (i = 0; i < n_alpha_; i++) {
      for(j = 0; j < n_alpha_; j++) {
        if(IsUpperBounded(j)) // alpha_j >= C
          grad_bar_[i] = grad_bar_[i] + GetC_(j) * y_[j] * CalcKernelValue_(i,j);
      }

      grad_bar_[i] = y_[i] * grad_bar_[i];
    }
  }

  Log::Info << "SMO initialization done." << std::endl;

  // Begin SMO iterations
  ct_iter_ = 0;

  int stop_condition = 0;
  while (1) {
    //for(size_t i=0; i<n_alpha_; i++)
    //  printf("%f.\n", y_[i]*alpha_[i]);
    //printf("\n\n");

    // for every min(n_data_, 1000) iterations, do shrinking
    if (do_shrinking_) {
      if (--ct_shrinking_ == 0) {
        Shrinking_();
        ct_shrinking_ = std::min(n_data_, SMO_NUM_FOR_SHRINKING);
      }
    }

    // Find working set, check stopping criterion, update gradient and alphas
    stop_condition = SMOIterations_();
    // Termination check, if stop_condition==1 or ==2 => SMO terminates
    if (stop_condition == 1) {// optimality reached
      // Calculate the bias term
      CalcBias_();
      Log::Info << "SMO has achieved accuracy " << accuracy_
          << " in " << ct_iter_ << " iterations and can now terminate."
          << std::endl;
      break;
    }
    else if (stop_condition == 2) {// max num of iterations exceeded
      // Calculate the bias term
      CalcBias_();
      Log::Warn << "SMO has exceeded " << ct_iter_ << " iterations.  Gap: "
          << gap_ << "." << std::endl;
      break;
    }
  }
}

/**
* SMO training iterations
*
* @return: stopping condition id
*/
template<typename TKernel>
int SMO<TKernel>::SMOIterations_() {
  ct_iter_ ++;
  size_t i,j;
  if (WorkingSetSelection_(i,j) == true) {
    if (!do_shrinking_) { // no shrinking, optimality reached
      return 1;
    } else { // shrinking, need to check whether optimality really reached
      ReconstructGradient_(); // restore the inactive alphas and reconstruct gradients
      n_active_ = n_alpha_;
      if (WorkingSetSelection_(i,j) == true) { // optimality reached
        return 1;
      }
      else {
        ct_shrinking_ = 1; // do shrinking in the next iteration
        return 0;
      }
    }
  } else if (ct_iter_ >= n_iter_) { // number of iterations exceeded
    if (!do_shrinking_) { // no shrinking, optimality reached
      return 2;
    }
    else if ( ct_iter_ >= std::min(n_data_, SMO_NUM_FOR_SHRINKING) ) { // shrinking has been carried out, need to calculate the true gap
      ReconstructGradient_(); // restore the inactive alphas and reconstruct gradients
      n_active_ = n_alpha_;
      WorkingSetSelection_(i,j);
      return 2;
    }
    else {
      return 2;
    }
  } else { // update gradient, alphas and bias term, and continue iterations
    UpdateGradientAlpha_(i, j);
    return 0;
  }
}

/**
* Try to find a working set (i,j). Both 1st(default) and 2nd order approximations of
* the objective function Z(\alpha+\lambda u_ij)-Z(\alpha) are implemented.
*
* @param: reference to working set (i, j)
*
* @return: working set (i, j); indicator of whether the optimal solution is reached (true:reached)
*/
template<typename TKernel>
bool SMO<TKernel>::WorkingSetSelection_(size_t &out_i, size_t &out_j) {
  double y_grad_max = -INFINITY;
  double y_grad_min =  INFINITY;
  int idx_i = -1;
  int idx_j = -1;

  // Find i using maximal violating pair scheme
  size_t t;
  for (t = 0; t < n_active_; t++) { // find argmax(y*grad), t\in I_up
    if (y_[t] == 1) {
      if (!IsUpperBounded(t)) {// t\in I_up, y==1: y[t]alpha[t] < C
        if (grad_[t] > y_grad_max) { // y==1
          y_grad_max = grad_[t];
          idx_i = t;
        }
      }
    } else { // y[t] == -1
      if (!IsLowerBounded(t)) { // t\in I_up, y==-1: y[t]alpha[t] < 0
        if (grad_[t] + y_grad_max < 0) { // y==-1... <=> -grad_[t] > y_grad_max
          y_grad_max = -grad_[t];
          idx_i = t;
        }
      }
    }
  }

  out_i = idx_i; // i found

  /*  Find j using maximal violating pair scheme (1st order approximation) */
  if (wss_ == 1) {
    for (t = 0; t < n_active_; t++) { // find argmin(y*grad), t\in I_down
      if (y_[t] == 1) {
        if (!IsLowerBounded(t)) { // t\in I_down, y==1: y[t]alpha[t] > 0
          if (grad_[t] < y_grad_min) { // y==1
            y_grad_min = grad_[t];
            idx_j = t;
          }
        }
      } else { // y[t] == -1
        if (!IsUpperBounded(t)) { // t\in I_down, y==-1: y[t]alpha[t] > -C
          if (grad_[t] + y_grad_min > 0) { // y==-1...<=>  -grad_[t] < y_grad_min
            y_grad_min = -grad_[t];
            idx_j = t;
          }
        }
      }
    }

    out_j = idx_j; // j found

  } else if (wss_ == 2) {

    /* Find j using 2nd order working set selection scheme; need to calc kernels, but faster convergence */
    double K_ii = CalcKernelValue_(out_i, out_i);
    double opt_gain_max = -INFINITY;
    double grad_diff;
    double quad_kernel;
    double opt_gain = -INFINITY;

    for (t = 0; t < n_active_; t++) {
      double K_it = CalcKernelValue_(out_i, t);
      double K_tt = CalcKernelValue_(t, t);

      if (y_[t] == 1) {
        if (!IsLowerBounded(t)) { // t\in I_down, y==1: y[t]alpha[t] > 0
          // calculate y_grad_min for Stopping Criterion
          if (grad_[t] < y_grad_min) // y==1
            y_grad_min = grad_[t];

          // find j
          grad_diff = y_grad_max - grad_[t]; // max(y_i*grad_i) - y_t*grad_t
          if (grad_diff > 0) {
            quad_kernel = K_ii + K_tt - 2 * K_it;

            if (quad_kernel > 0) // for positive definite kernels
              opt_gain = ( grad_diff * grad_diff ) / quad_kernel; // actually ../2*quad_kernel
            else // handle non-positive definite kernels
              opt_gain = ( grad_diff * grad_diff ) / TAU;

            // find max(opt_gain)
            if (opt_gain > opt_gain_max) {
              idx_j = t;
              opt_gain_max = opt_gain;
            }
          }
        }
      } else { // y[t] == -1
        if (!IsUpperBounded(t)) { // t\in I_down, y==-1: y[t]alpha[t] > -C
          // calculate y_grad_min for Stopping Criterion
          if (grad_[t] + y_grad_min > 0) // y==-1, -grad_[t] < y_grad_min
            y_grad_min = -grad_[t];

          // find j
          grad_diff = y_grad_max + grad_[t]; // max(y_i*grad_i) - y_t*grad_t

          if (grad_diff > 0) {
            quad_kernel = K_ii + K_tt - 2 * K_it;

            if (quad_kernel > 0) // for positive definite kernels
              opt_gain = ( grad_diff * grad_diff ) / quad_kernel; // actually ../2*quad_kernel
            else // handle non-positive definite kernels
              opt_gain = ( grad_diff * grad_diff ) / TAU;

            // find max(opt_gain)
            if (opt_gain > opt_gain_max) {
              idx_j = t;
              opt_gain_max = opt_gain;
            }
          }
        }
      }
    }
  }

  out_j = idx_j; // j found

  //printf("y_i=%d, y_j=%d\n", y_[out_i], y_[out_j]);
  //printf("a_i=%f, a_j=%f\n", alpha_[out_i], alpha_[out_j]);

  // Stopping Criterion check
  //printf("ct_iter:%d, accu:%f\n", ct_iter_, y_grad_max - y_grad_min);

  gap_ = y_grad_max - y_grad_min;

  //printf("%d: gap=%f\n", ct_iter_, gap_);

  return (gap_ <= accuracy_); // If gap_ <= accuracy, optimality was reached.
}

/**
* Search direction; Update gradient, alphas and bias term
*
* @param: a working set (i,j) found by working set selection
*
*/
template<typename TKernel>
void SMO<TKernel>::UpdateGradientAlpha_(size_t i, size_t j) {
  size_t t;

  double a_i = alpha_[i]; // old alphas
  double a_j = alpha_[j];
  int y_i = y_[i];
  int y_j = y_[j];
  double C_i = GetC_(i); // can be Cp (for y==1) or Cn (for y==-1)
  double C_j = GetC_(j);

  // cached kernel values
  double K_ii, K_ij, K_jj;
  K_ii = CalcKernelValue_(i, i);
  K_ij = CalcKernelValue_(i, j);
  K_jj = CalcKernelValue_(j, j);

  double first_order_diff = y_i * grad_[i] - y_j * grad_[j];
  double second_order_diff = K_ii + K_jj - 2 * K_ij;
  if (second_order_diff <= 0) // handle non-positive definite kernels
    second_order_diff = TAU;
  double lambda = first_order_diff / second_order_diff; // step size

  //printf("step size=%f\n", lambda);

  /*
  double step_B, step_A;
  if (y_i == 1) {
    step_B = C_i - a_i;
  }
  else { // y_i == -1
    step_B = a_i; // 0-(-1)a_i
  }
  if (y_j == 1) {
    step_A = a_j;
  }
  else { // y_j == -1
    step_A = C_j - a_j; // (-1)a_j - (-C_j)
  }
  double min_step_temp = min(step_B, step_A);
  double min_step = min(min_step_temp, newton_step);
  */

  // Update alphas
  alpha_[i] = a_i + y_i * lambda;
  alpha_[j] = a_j - y_j * lambda;

  // Update alphas and handle bounds for updated alphas
  /*
  if (y_i != y_j) {
    double alpha_old_diff = a_i - a_j;
    if (alpha_old_diff > 0) {
      if (alpha_[i] < alpha_old_diff) {
        alpha_[i] = alpha_old_diff;
      }
      else if (alpha_[i] > C_i) {
        alpha_[i] = C_i;
      }
    }
    else { // alpha_old_diff <= 0
      if (alpha_[i] < 0) {
        alpha_[i] = 0;
      }
      else if (alpha_[i] > C_i + alpha_old_diff) {
        alpha_[i] = C_i + alpha_old_diff;
      }
    }
  }
  else { // y_i == y_j
    double alpha_old_sum = a_i + a_j;
    if (alpha_old_sum > C_i) {
      if (alpha_[i] < alpha_old_sum - C_i) {
        alpha_[i] =  alpha_old_sum - C_i;
      }
      else if (alpha_[i] > C_i) {
        alpha_[i] = C_i;
      }
    }
    else { //alpha_old_sum <= C_i
      if (alpha_[i] < 0) {
        alpha_[i] = 0;
      }
      else if (alpha_[i] > alpha_old_sum) {
        alpha_[i] = alpha_old_sum;
      }
    }
  }
  alpha_[j] = a_j + y_i * y_j * (a_i - alpha_[i]);
  */

  // Handle bounds for updated alphas
  if (y_i != y_j) {
    double alpha_old_diff = a_i - a_j;
    if (alpha_old_diff > 0) {
      if (alpha_[j] < 0) {
        alpha_[j] = 0;
        alpha_[i] = alpha_old_diff;
      }
    }
    else { // alpha_old_diff <= 0
      if (alpha_[i] < 0) {
        alpha_[i] = 0;
        alpha_[j] = - alpha_old_diff;
      }
    }
    if (alpha_old_diff > C_i - C_j) {
      if (alpha_[i] > C_i) {
        alpha_[i] = C_i;
        alpha_[j] = C_i - alpha_old_diff;
      }
    }
    else {
      if (alpha_[j] > C_j) {
        alpha_[j] = C_j;
        alpha_[i] = C_j + alpha_old_diff;
      }
    }
  }
  else { // y_i == y_j
    double alpha_old_sum = a_i + a_j;
    if (alpha_old_sum > C_i) {
      if (alpha_[i] > C_i) {
        alpha_[i] = C_i;
        alpha_[j] = alpha_old_sum - C_i;
      }
    }
    else {
      if (alpha_[j] < 0) {
        alpha_[j] = 0;
        alpha_[i] = alpha_old_sum;
      }
    }
    if (alpha_old_sum > C_j) {
      if (alpha_[j] > C_j) {
        alpha_[j] = C_j;
        alpha_[i] = alpha_old_sum - C_j;
      }
    }
    else {
      if (alpha_[i] < 0) {
        alpha_[i] = 0;
        alpha_[j] = alpha_old_sum;
      }
    }
  }

  // Update gradient
  double diff_i = alpha_[i] - a_i;
  double diff_j = alpha_[j] - a_j;
  for (t=0; t<n_active_; t++) {
    grad_[t] = grad_[t] - y_[t] * (y_[i] * diff_i * CalcKernelValue_(i, t) + y_[j] * diff_j * CalcKernelValue_(j, t));
  }

  bool ub_i = IsUpperBounded(i);
  bool ub_j = IsUpperBounded(j);

  // Update alpha active status
  UpdateAlphaStatus_(i);
  UpdateAlphaStatus_(j);

  if (do_shrinking_) {
    // Update gradient_bar
    if(ub_i != IsUpperBounded(i) ) { // updated_alpha_i >= C
      if (ub_i) // old_alpha_i >= C, new_alpha_i < C
        for(t = 0; t < n_alpha_; t++)
          grad_bar_[t] = grad_bar_[t] - C_i * y_[i] * y_[t] * CalcKernelValue_(i, t);
      else // old_alpha_i < C, new_alpha_i >= C
        for(t = 0; t < n_alpha_; t++)
          grad_bar_[t] = grad_bar_[t] + C_i * y_[i] * y_[t] * CalcKernelValue_(i, t);
    }
    if (ub_j != IsUpperBounded(j) ) {
      if (ub_j) // old_alpha_j >= C, new_alpha_j < C
        for (t = 0; t < n_alpha_; t++)
          grad_bar_[t] = grad_bar_[t] - C_j * y_[j] * y_[t] * CalcKernelValue_(j, t);
      else // old_alpha_j < C, new_alpha_j >= C
        for (t = 0; t < n_alpha_; t++)
          grad_bar_[t] = grad_bar_[t] + C_j * y_[j] * y_[t] * CalcKernelValue_(j, t);
    }
  }

}

/**
* Calcualte bias term
*
* @return: the bias
*
*/
template<typename TKernel>
void SMO<TKernel>::CalcBias_() {
  double b;
  size_t n_free_alpha = 0;
  double ub = INFINITY, lb = -INFINITY, sum_free_yg = 0.0;

  for (size_t i=0; i<n_active_; i++){
    double yg = y_[i] * grad_[i];

    if (IsUpperBounded(i)) { // bounded: alpha_i >= C
      if(y_[i] == 1)
        lb = std::max(lb, yg);
      else
        ub = std::min(ub, yg);
    }
    else if (IsLowerBounded(i)) { // bounded: alpha_i <= 0
      if(y_[i] == -1)
        lb = std::max(lb, yg);
      else
        ub = std::min(ub, yg);
    }
    else { // free: 0< alpha_i <C
      n_free_alpha++;
      sum_free_yg += yg;
    }
  }

  if(n_free_alpha > 0)
    b = sum_free_yg / n_free_alpha;
  else
    b = (ub + lb) / 2;

  bias_ = b;
}

/* Get SVM results:coefficients, number and indecies of SVs
*
* @param: sample indices of the training (sub)set in the total training set
* @param: support vector coefficients: alpha*y
* @param: bool indicators  FOR THE TRAINING SET: is/isn't a support vector
*
*/
template<typename TKernel>
void SMO<TKernel>::GetSV(std::vector<size_t> &dataset_index, std::vector<double> &coef, std::vector<bool> &sv_indicator) {
  std::vector<size_t> new_from_old; // it's used to retrieve the permuted new index from old index
  new_from_old.resize(n_alpha_, 0);
  for (size_t i = 0; i < n_alpha_; i++) {
    new_from_old[active_set_[i]] = i;
  }
  if (learner_typeid_ == 0) {// SVM_C
    for (size_t ii = 0; ii < n_data_; ii++) {
      size_t i = new_from_old[ii]; // retrive the index of permuted vector
      if (alpha_[i] >= SMO_ALPHA_ZERO) { // support vectors found
        //printf("%f\n", alpha_[i] * y_[i]);
        coef.push_back(alpha_[i] * y_[i]);
        sv_indicator[dataset_index[ii]] = true;
        n_sv_++;
      }
      else {
        coef.push_back(0);
      }
    }

    Log::Info << "Number of support vectors: " << n_sv_ << "." << std::endl;

  }
  else if (learner_typeid_ == 1) {// SVM_R
    for (size_t ii = 0; ii < n_data_; ii++) {
      size_t i = new_from_old[ii]; // retrive the index of permuted vector
      size_t iplusn = new_from_old[ii+n_data_];
      double alpha_diff = -alpha_[i] + alpha_[iplusn]; // alpha_i^* - alpha_i
      if (fabs(alpha_diff) >= SMO_ALPHA_ZERO) { // support vectors found
        coef.push_back(alpha_diff);
        sv_indicator[dataset_index[ii]] = true;
        n_sv_++;
      } else {
        coef.push_back(0);
      }
    }
  }
}

}; // namespace svm
}; // namespace mlpack

#endif
