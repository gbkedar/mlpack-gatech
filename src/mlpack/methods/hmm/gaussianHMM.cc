/**
 * @file gaussianHMM.cc
 *
 * This file contains the implementation of functions in gaussianHMM.h
 */
#include <mlpack/core.h>

#include "support.h"
#include "gaussianHMM.h"

using namespace hmm_support;

void GaussianHMM::setModel(const arma::mat& transmission,  const std::vector<arma::vec>& list_mean_vec, const std::vector<arma::mat>& list_covariance_mat) {
  mlpack::IO::Assert(transmission.n_rows == transmission.n_cols);
  mlpack::IO::Assert(transmission.n_rows == list_mean_vec.size());
  mlpack::IO::Assert(transmission.n_rows == list_covariance_mat.size());

  for (size_t i = 1; i < list_mean_vec.size(); i++) {
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_covariance_mat[i].n_rows);
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_covariance_mat[i].n_cols);
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_mean_vec[i].n_elem);
  }

  transmission_ = transmission;
  list_mean_vec_ = list_mean_vec;
  list_covariance_mat_ = list_covariance_mat;
  CalculateInverse();
}

void GaussianHMM::Init(const arma::mat& transmission, const std::vector<arma::vec>& list_mean_vec,const std::vector<arma::mat>& list_covariance_mat) {
  transmission_ = transmission;
  list_mean_vec_ = list_mean_vec;
  list_covariance_mat_ = list_covariance_mat;

  mlpack::IO::Assert(transmission.n_rows == transmission.n_cols);
  mlpack::IO::Assert(transmission.n_rows == list_mean_vec.size());
  mlpack::IO::Assert(transmission.n_rows == list_covariance_mat.size());
  for (size_t i = 1; i < list_mean_vec.size(); i++) {
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_covariance_mat[i].n_rows);
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_covariance_mat[i].n_cols);
    mlpack::IO::Assert(list_mean_vec[0].n_elem == list_mean_vec[i].n_elem);
  }

  CalculateInverse();
}

void GaussianHMM::InitFromFile(const char* profile) {
  if (!(GaussianHMM::LoadProfile(profile, transmission_, list_mean_vec_, list_covariance_mat_)))
    mlpack::IO::Fatal << "Couldn't open " << profile << " for reading." <<
        std::endl;

  list_inverse_cov_mat_ = list_covariance_mat_;
  gauss_const_vec_.set_size(list_covariance_mat_.size());

  CalculateInverse();
}

void GaussianHMM::InitFromData(const std::vector<arma::mat>& list_data_seq, size_t numstate) {
  GaussianHMM::InitGaussParameter(numstate, list_data_seq, transmission_, list_mean_vec_, list_covariance_mat_);

  list_inverse_cov_mat_ = list_covariance_mat_;
  gauss_const_vec_.set_size(list_covariance_mat_.size());

  CalculateInverse();
}

void GaussianHMM::InitFromData(const arma::mat& data_seq, const arma::vec& state_seq)  {
  GaussianHMM::EstimateInit(data_seq, state_seq, transmission_, list_mean_vec_, list_covariance_mat_);
  CalculateInverse();
}

void GaussianHMM::LoadProfile(const char* profile) {
  /*
  transmission_.Destruct();
  list_mean_vec_.Renew();
  list_covariance_mat_.Renew();
  list_inverse_cov_mat_.Renew();
  gauss_const_vec_.Destruct();
  */
  InitFromFile(profile);
}

void GaussianHMM::SaveProfile(const char* profile) const {
  GaussianHMM::SaveProfile(profile, transmission_, list_mean_vec_, list_covariance_mat_);
}

void GaussianHMM::CalculateInverse() {
  size_t M = transmission_.n_rows;
  size_t N = list_mean_vec_[0].n_elem;
  for (size_t i = 0; i < M; i++) {
    list_inverse_cov_mat_[i] = inv(list_covariance_mat_[i]); // inverse
    gauss_const_vec_[i] = pow(2.0 * math::PI, -N / 2.0) * pow(det(list_covariance_mat_[i]), -0.5);
  }
}

void GaussianHMM::GenerateSequence(size_t L, arma::mat& data_seq, arma::vec& state_seq) const {
  GaussianHMM::GenerateInit(L, transmission_, list_mean_vec_, list_covariance_mat_, data_seq, state_seq);
}

void GaussianHMM::EstimateModel(const arma::mat& data_seq, const arma::vec& state_seq) {
  GaussianHMM::EstimateInit(data_seq, state_seq, transmission_, list_mean_vec_, list_covariance_mat_);
  CalculateInverse();
}

void GaussianHMM::EstimateModel(size_t numstate, const arma::mat& data_seq, const arma::vec& state_seq) {
  GaussianHMM::EstimateInit(numstate, data_seq, state_seq, transmission_, list_mean_vec_, list_covariance_mat_);
  CalculateInverse();
}

void GaussianHMM::DecodeOverwrite(const arma::mat& data_seq, arma::mat& state_prob_mat,
				  arma::mat& forward_prob_mat, arma::mat& backward_prob_mat, arma::vec& scale_vec) const {
  size_t M = transmission_.n_rows;
  size_t L = data_seq.n_cols;
  arma::mat emission_prob_mat(M, L);
  GaussianHMM::CalculateEmissionProb(data_seq, list_mean_vec_, list_inverse_cov_mat_,
				     gauss_const_vec_, emission_prob_mat);
  GaussianHMM::Decode(transmission_, emission_prob_mat, state_prob_mat, forward_prob_mat,
		      backward_prob_mat, scale_vec);
}

void GaussianHMM::DecodeInit(const arma::mat& data_seq, arma::mat& state_prob_mat,
			     arma::mat& forward_prob_mat, arma::mat& backward_prob_mat, arma::vec& scale_vec) const {
  size_t M = transmission_.n_rows;
  size_t L = data_seq.n_cols;

  state_prob_mat.set_size(M, L);
  forward_prob_mat.set_size(M, L);
  backward_prob_mat.set_size(M, L);
  scale_vec.set_size(L);

  arma::mat emission_prob_mat(M, L);

  GaussianHMM::CalculateEmissionProb(data_seq, list_mean_vec_, list_inverse_cov_mat_,
				     gauss_const_vec_, emission_prob_mat);
  GaussianHMM::Decode(transmission_, emission_prob_mat, state_prob_mat, forward_prob_mat,
		      backward_prob_mat, scale_vec);
}

double GaussianHMM::ComputeLogLikelihood(const arma::mat& data_seq) const {
  size_t L = data_seq.n_cols;
  size_t M = transmission_.n_rows;

  arma::mat fs(M, L), emis_prob(M, L);
  arma::vec sc(L);

  GaussianHMM::CalculateEmissionProb(data_seq, list_mean_vec_, list_inverse_cov_mat_, gauss_const_vec_, emis_prob);
  GaussianHMM::ForwardProcedure(L, transmission_, emis_prob, sc, fs);
  double loglik = 0;
  for (size_t t = 0; t < L; t++)
    loglik += log(sc[t]);
  return loglik;
}

void GaussianHMM::ComputeLogLikelihood(const std::vector<arma::mat>& list_data_seq, std::vector<double>& list_likelihood) const {
  size_t L = 0;
  for (size_t i = 0; i < list_data_seq.size(); i++) {
    if (list_data_seq[i].n_cols > L)
      L = list_data_seq[i].n_cols;
  }

  size_t M = transmission_.n_rows;

  arma::mat fs(M, L), emis_prob(M, L);
  arma::vec sc(L);

  for (size_t i = 0; i < list_data_seq.size(); i++) {
    size_t L = list_data_seq[i].n_cols;

    GaussianHMM::CalculateEmissionProb(list_data_seq[i], list_mean_vec_, list_inverse_cov_mat_, gauss_const_vec_, emis_prob);
    GaussianHMM::ForwardProcedure(L, transmission_, emis_prob, sc, fs);

    double loglik = 0;
    for (size_t t = 0; t < L; t++)
      loglik += log(sc[t]);
    list_likelihood.push_back(loglik);
  }
}

void GaussianHMM::ComputeViterbiStateSequence(const arma::mat& data_seq, arma::vec& state_seq) const {
  size_t M = transmission_.n_rows;
  size_t L = data_seq.n_cols;

  arma::mat emis_prob(M, L);

  GaussianHMM::CalculateEmissionProb(data_seq, list_mean_vec_, list_inverse_cov_mat_, gauss_const_vec_, emis_prob);
  GaussianHMM::ViterbiInit(transmission_, emis_prob, state_seq);
}

void GaussianHMM::TrainBaumWelch(const std::vector<arma::mat>& list_data_seq, size_t max_iteration, double tolerance) {
  GaussianHMM::Train(list_data_seq, transmission_, list_mean_vec_, list_covariance_mat_, max_iteration, tolerance);
  CalculateInverse();
}

void GaussianHMM::TrainViterbi(const std::vector<arma::mat>& list_data_seq, size_t max_iteration, double tolerance) {
  GaussianHMM::TrainViterbi(list_data_seq, transmission_, list_mean_vec_, list_covariance_mat_, max_iteration, tolerance);
  CalculateInverse();
}

bool GaussianHMM::LoadProfile(const char* profile, arma::mat& trans, std::vector<arma::vec>& means, std::vector<arma::mat>& covs) {
  std::vector<arma::mat> matlst;
  if (!(load_matrix_list(profile, matlst)))
    return false;

  mlpack::IO::Assert(matlst.size() > 0);

  trans = matlst[0];
  size_t M = trans.n_rows; // num of states
  mlpack::IO::Assert(matlst.size() == (2 * M + 1));
  size_t N = matlst[1].n_rows; // dimension

  for (size_t i = 1; i < (2 * M + 1); i += 2) {
    mlpack::IO::Assert(matlst[i].n_rows == N && matlst[i].n_cols == 1);
    mlpack::IO::Assert(matlst[i + 1].n_rows == N && matlst[i + 1].n_cols == N);

    arma::vec m = matlst[i].unsafe_col(0);

    means.push_back(m);
    covs.push_back(matlst[i + 1]);
  }

  return true;
}

bool GaussianHMM::SaveProfile(const char* profile, const arma::mat& trans, const std::vector<arma::vec>& means, const std::vector<arma::mat>& covs) {
  TextWriter w_pro;
  if (!(w_pro.Open(profile))) {
    mlpack::IO::Warn << "Couldn't open " << profile << " for writing." <<
        std::endl;
    return false;
  }

  size_t M = trans.n_rows; // num of states
  mlpack::IO::Assert(means.size() == M && covs.size() == M);
  size_t N = means[0].n_elem; // dimension
  print_matrix(w_pro, trans, "% transmission", "%f,");
  for (size_t i = 0; i < M; i++) {
    mlpack::IO::Assert(means[i].n_elem == N);
    mlpack::IO::Assert(covs[i].n_rows == N && covs[i].n_cols == N);
    char s[100];
    sprintf(s, "%% mean - state %zu", i);
    print_vector(w_pro, means[i], s, "%f,");
    sprintf(s, "%% covariance - state%zu", i);
    print_matrix(w_pro, covs[i], s, "%f,");
  }

  return true;
}

void GaussianHMM::GenerateInit(size_t L, const arma::mat& trans, const std::vector<arma::vec>& means, const std::vector<arma::mat>& covs, arma::mat& seq, arma::vec& states){
  mlpack::IO::AssertMessage((trans.n_rows == trans.n_cols && trans.n_rows == means.size() && trans.n_rows == covs.size()),
      "GaussianHMM::GenerateInit(): matrix sizes do not match");

  arma::mat trsum;
  size_t M, N;
  size_t cur_state;

  M = trans.n_rows;
  N = means[0].n_elem;  // emission vector length

  trsum = trans;

  for (size_t i = 0; i < M; i++) {
    for (size_t j = 1; j < M; j++) {
      trsum(i, j) += trsum(i, j - 1);
    }
  }

  seq.set_size(N, L);
  states.set_size(L);

  cur_state = 0; // starting state is 0

  for (size_t i = 0; i < L; i++) {
    size_t j;

    // next state
    double r = (double) rand() / (double) RAND_MAX;
    for (j = 0; j < M; j++) {
      if (r <= trsum(cur_state, j))
        break;
    }
    cur_state = j;

    // emission
    arma::vec e;
    RAND_NORMAL_INIT(means[cur_state], covs[cur_state], e);
    for (j = 0; j < N; j++)
      seq(j, i) = e[j];
    states[i] = cur_state;
  }
}

void GaussianHMM::EstimateInit(const arma::mat& seq, const arma::vec& states, arma::mat& trans, std::vector<arma::vec>& means, std::vector<arma::mat>& covs) {
  mlpack::IO::AssertMessage((seq.n_cols == states.n_elem), "GaussianHMM::EstimateInit(): sequence and states length must be the same");

  size_t M = 0;
  for (size_t i = 0; i < seq.n_cols; i++) {
    if (states[i] > M)
      M = (size_t) states[i];
  }
  M++;
  GaussianHMM::EstimateInit(M, seq, states, trans, means, covs);
}

void GaussianHMM::EstimateInit(size_t numStates, const arma::mat& seq, const arma::vec& states, arma::mat& trans, std::vector<arma::vec>& means, std::vector<arma::mat>& covs) {
  mlpack::IO::AssertMessage((seq.n_cols == states.n_elem), "GaussianHMM::EstimateInit(): sequence and states length must be the same");

  size_t N = seq.n_rows; // emission vector length
  size_t M = numStates;  // number of states
  size_t L = seq.n_cols; // sequence length

  arma::vec stateSum;

  trans.zeros(M, M);
  stateSum.zeros(M);

  for (size_t i = 0; i < M; i++) {
    arma::vec m;
    m.zeros(N);
    means.push_back(m);

    arma::mat c;
    c.zeros(N, N);
    covs.push_back(c);
  }

  for (size_t i = 0; i < L - 1; i++) {
    size_t state = (size_t) states[i];
    size_t next_state = (size_t) states[i + 1];
    stateSum[state]++;
    trans(state, next_state)++;
  }

  for (size_t i = 0; i < M; i++) {
    if (stateSum[i] == 0)
      stateSum[i] = -INFINITY;

    for (size_t j = 0; j < M; j++)
      trans(i, j) /= stateSum[i];
  }

  stateSum.zeros();
  for (size_t i = 0; i < L; i++) {
    size_t state = (size_t) states[i];
    arma::vec e = seq.unsafe_col(i);

    stateSum[state]++;
    means[state] += e;
  }

  for (size_t i = 0; i < M; i++) {
    if (stateSum[i] != 0)
      means[i] /= stateSum[i];
  }

  for (size_t i = 0; i < L; i++) {
    size_t state = (size_t) states[i];
    arma::vec e = seq.unsafe_col(i);
    arma::vec d = means[state] - e;
    covs[state] += d * arma::trans(d);
  }

  for (size_t i = 0; i < M; i++) {
    if (stateSum[i] != 0)
      covs[i] /= stateSum[i];
  }
}

void GaussianHMM::ForwardProcedure(size_t L, const arma::mat& trans, const arma::mat& emis_prob, arma::vec& scales, arma::mat& fs) {
  size_t M = trans.n_rows;

  fs.zeros();
  scales.zeros();

  // NOTE: start state is 0
  // time t = 0
  for (size_t i = 0; i < M; i++) {
    fs(i, 0) = trans(0, i) * emis_prob(i, 0);
    scales[0] += fs(i, 0);
  }

  for (size_t i = 0; i < M; i++)
    fs(i, 0) /= scales[0];

  // time t = 1 -> L-1
  for (size_t t = 1; t < L; t++) {
    for (size_t j = 0; j < M; j++) {
      for (size_t i = 0; i < M; i++)
	fs(j, t) += fs(i, t - 1) * trans(i, j);
      fs(j, t) *= emis_prob(j, t);
      scales[t] += fs(j, t);
    }
    for (size_t j = 0; j < M; j++)
      fs(j, t) /= scales[t];
  }
}

void GaussianHMM::BackwardProcedure(size_t L, const arma::mat& trans, const arma::mat& emis_prob, const arma::vec& scales, arma::mat& bs) {
  size_t M = trans.n_rows;

  bs.zeros();

  for (size_t i = 0; i < M; i++)
    bs(i, L - 1) = 1.0;

  for (size_t t = L - 2; t >= 0; t--) {
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < M; j++)
	bs(i, t) += trans(i, j) * bs(j, t + 1) * emis_prob(j, t + 1);
      bs(i, t) /= scales[t + 1];
    }
  }
}

double GaussianHMM::Decode(size_t L, const arma::mat& trans, const arma::mat& emis_prob, arma::mat& pstates, arma::mat& fs, arma::mat& bs, arma::vec& scales) {
  size_t M = trans.n_rows;

  mlpack::IO::AssertMessage((L == pstates.n_cols && L == fs.n_cols && L == bs.n_cols &&
		    M == trans.n_cols && M == emis_prob.n_rows),
                    "GaussianHMM::Decode(): sizes do not match");

  GaussianHMM::ForwardProcedure(L, trans, emis_prob, scales, fs);
  GaussianHMM::BackwardProcedure(L, trans, emis_prob, scales, bs);

  for (size_t i = 0; i < M; i++)
    for (size_t t = 0; t < L; t++)
      pstates(i, t) = fs(i,t) * bs(i,t);

  double logpseq = 0;
  for (size_t t = 0; t < L; t++)
    logpseq += log(scales[t]);

  return logpseq;
}

double GaussianHMM::Decode(const arma::mat& trans, const arma::mat& emis_prob, arma::mat& pstates, arma::mat& fs, arma::mat& bs, arma::vec& scales) {
  size_t L = emis_prob.n_cols;
  return GaussianHMM::Decode(L, trans, emis_prob, pstates, fs, bs, scales);
}

double GaussianHMM::ViterbiInit(const arma::mat& trans, const arma::mat& emis_prob, arma::vec& states) {
  size_t L = emis_prob.n_cols;
  return GaussianHMM::ViterbiInit(L, trans, emis_prob, states);
}

double GaussianHMM::ViterbiInit(size_t L, const arma::mat& trans, const arma::mat& emis_prob, arma::vec& states) {
  size_t M = trans.n_rows;
  mlpack::IO::AssertMessage((M == trans.n_cols && M == emis_prob.n_rows),
      "GaussianHMM::ViterbitInit(): sizes do not match");

  states.set_size(L);

  arma::vec v(M), v_old;
  v.fill(-INFINITY);
  v[0] = 0;
  v_old = v;

  arma::mat w(M, L);
  arma::mat logtrans(M, M);

  for (size_t i = 0; i < M; i++) {
    for (size_t j = 0; j < M; j++)
      logtrans(i, j) = log(trans(i, j));
  }


  for (size_t t = 0; t < L; t++) {
    for (size_t j = 0; j < M; j++) {
      double bestVal = -INFINITY;
      double bestPtr = -1;
      for (size_t i = 0; i < M; i++) {
	double val = v_old[i] + logtrans(i, j);
	if (val > bestVal) {
	  bestVal = val;
	  bestPtr = i;
	}
      }
      v[j] = bestVal + log(emis_prob(j, t));
      w(j, t) = bestPtr;
    }
    v_old = v;
  }

  double bestVal = -INFINITY;
  double bestPtr = -1;
  for (size_t i = 0; i < M; i++)
    if (v[i] > bestVal) {
      bestVal = v[i];
      bestPtr = i;
    }

  states[L - 1] = bestPtr;
  for (size_t t = L - 2; t >= 0; t--)
    states[t] = w((size_t) states[t + 1], t + 1);

  return bestVal;
}

void GaussianHMM::CalculateEmissionProb(const arma::mat& seq, const std::vector<arma::vec>& means, const std::vector<arma::mat>& inv_covs, const arma::vec& det, arma::mat& emis_prob) {
  size_t L = seq.n_cols;
  size_t M = means.size();
  for (size_t t = 0; t < L; t++) {
    arma::vec e = seq.unsafe_col(t);
    for (size_t i = 0; i < M; i++)
      emis_prob(i, t) = NORMAL_DENSITY(e, means[i], inv_covs[i], det[i]);
  }
}

void GaussianHMM::InitGaussParameter(size_t M, const std::vector<arma::mat>& seqs, arma::mat& guessTR, std::vector<arma::vec>& guessME, std::vector<arma::mat>& guessCO) {
  size_t N = seqs[0].n_rows;

  std::vector<size_t> labels;
  arma::vec sumState;

  kmeans(seqs, M, labels, guessME, 1000, 1e-5);

  //for (size_t i = 0; i < labels.size(); i++) printf("%8d", labels[i]);
  //printf("---1---\n");

  guessTR.zeros(M, M);
  sumState.zeros(M);
  for (size_t i = 0; i < M; i++) {
    arma::mat m;
    m.zeros(N, N);
    guessCO.push_back(m);
  }
  //printf("---2---\n");

  size_t t = 0;
  for (size_t p = 0; p < seqs.size(); p++) {
    for (size_t q = 0; q < seqs[p].n_cols; q++, t++) {
      if (q == seqs[p].n_cols - 1)
        continue;

      size_t i = labels[t];
      size_t j = labels[t + 1];

      guessTR(i, j)++;
      sumState[i]++;

      arma::vec data_j_Vec = seqs[p].unsafe_col(q);
      arma::mat tmp_cov;

      arma::vec sub_Vec = data_j_Vec - guessME[i];
      tmp_cov = sub_Vec;
      //printf("t = %d x = %8.3f\n", t, sub_Vec[0]);
      guessCO[i] += tmp_cov * trans(tmp_cov);
    }
  }
  //printf("---3---\n");

  for (size_t i = 0; i < M; i++)
    if (sumState[i] == 0) {
      for (size_t j = 0; j < M; j++)
        guessTR(i, j) = 0;

      guessTR(i, i) = 1;
      guessME[i].zeros();
      guessCO[i].zeros();

      for (size_t j = 0; j < N; j++)
        guessCO[i](j, j) = 1;
    }
    else {
      for (size_t j = 0; j < M; j++)
        guessTR(i, j) /= sumState[i];

      guessCO[i] /= sumState[i];

      for (size_t j = 0; j < N; j++)
        guessCO[i](j, j) += 1e-3; // make sure the diagonal elements are not too small
    }
  //printf("---4---\n");
}

void GaussianHMM::TrainViterbi(const std::vector<arma::mat>& seqs, arma::mat& guessTR, std::vector<arma::vec>& guessME, std::vector<arma::mat>& guessCO, size_t max_iter, double tol) {
  size_t L = -1;
  size_t M = guessTR.n_rows;
  size_t N = guessME[0].n_elem;
  mlpack::IO::AssertMessage((M == guessTR.n_cols && M == guessME.size() && M == guessCO.size()),
      "GaussianHMM::TrainViterbi(): sizes do not match");

  for (size_t i = 0; i < seqs.size(); i++) {
    if (seqs[i].n_cols > L)
      L = seqs[i].n_cols;
  }

  arma::mat TR(M, M); // accumulating transition
  std::vector<arma::vec> ME = guessME; // accumulating mean
  std::vector<arma::mat> CO = guessCO; // accumulating covariance
  std::vector<arma::mat> INV_CO = CO; // inverse matrix of the covariance
  arma::vec DET(M); // the determinant * constant of the Normal PDF formula

  arma::mat emis_prob(M, L);
  arma::vec sumState(M); // the denominator for each state

  double loglik = 0, oldlog;
  for (size_t iter = 0; iter < max_iter; iter++) {
    oldlog = loglik;
    loglik = 0;

    // set the accumulating values to zeros and compute the inverse matrices and determinant constants
    TR.zeros();
    for (size_t i = 0; i < M; i++) {
      ME[i].zeros();
      CO[i].zeros();
      INV_CO[i] = inv(guessCO[i]);
      DET[i] = pow(2.0 * math::PI, -N / 2.0) * pow(det(guessCO[i]), -0.5);
    }
    sumState.zeros();

    // for each sequence, we will use forward-backward procedure and then accumulate
    for (size_t idx = 0; idx < seqs.size(); idx++) {
      L = seqs[idx].n_cols;
      arma::vec states;
      GaussianHMM::CalculateEmissionProb(seqs[idx], guessME, INV_CO, DET, emis_prob); // first calculate the emission probabilities of the sequence
      loglik += GaussianHMM::ViterbiInit(L, guessTR, emis_prob, states); // get the most probable state sequence

      // accumulate expected transition & mean & covariance
      for (size_t t = 0; t < L - 1; t++) {
	size_t i = (size_t) states[t];
	size_t j = (size_t) states[t + 1];
	TR(i, j)++;
      }

      for (size_t t = 0; t < L; t++) {
	arma::vec e = seqs[idx].unsafe_col(t);
	size_t i = (size_t) states[t];
	sumState[i]++;
        ME[i] += e;

	arma::vec d = guessME[i] - e;
	arma::mat D = d;
        CO[i] += D * trans(D);
      }
      // end accumulate
    }

    // after accumulate all sequences: re-estimate transition & mean & covariance for the next iteration
    for (size_t i = 0; i < M; i++) {
      double s = 0;
      for (size_t j = 0; j < M; j++)
        s += TR(i, j);

      if (s == 0) {
	for (size_t j = 0; j < M; j++)
          guessTR(i, j) = 0;

	guessTR(i, i) = 1;
      } else {
	for (size_t j = 0; j < M; j++)
          guessTR(i, j) = TR(i, j) / s;
      }

      if (sumState[i] != 0) {
        guessME[i] = ME[i] / sumState[i];
        guessCO[i] = CO[i] / sumState[i];
      }
    }
    // end re-estimate

    printf("Iter = %zu Loglik = %8.4f\n", iter, loglik);
    if (fabs(oldlog - loglik) < tol) {
      printf("\nConverged after %zu iterations\n", iter);
      break;
    }
    oldlog = loglik;
  }
}


void GaussianHMM::Train(const std::vector<arma::mat>& seqs, arma::mat& guessTR, std::vector<arma::vec>& guessME, std::vector<arma::mat>& guessCO, size_t max_iter, double tol) {
  size_t L = -1;
  size_t M = guessTR.n_rows;
  size_t N = guessME[0].n_elem;

  mlpack::IO::AssertMessage((M == guessTR.n_cols && M == guessME.size() && M == guessCO.size()),
    "GaussianHMM::Train(): sizes do not match");

  for (size_t i = 0; i < seqs.size(); i++) {
    if (seqs[i].n_cols > L)
      L = seqs[i].n_cols;
  }

  arma::mat TR(M, M);; // guess transition and emission matrix
  std::vector<arma::vec> ME = guessME; // accumulating mean
  std::vector<arma::mat> CO = guessCO; // accumulating covariance
  std::vector<arma::mat> INV_CO = CO; // inverse matrix of the covariance
  arma::vec DET(M); // the determinant * constant of the Normal PDF formula

  arma::mat ps(M, L), fs(M, L), bs(M, L), emis_prob(M, L); // to hold hmm_decodeG results
  arma::vec s(L); // scaling factors
  arma::vec sumState(M); // the denominator for each state

  double loglik = 0, oldlog;
  for (size_t iter = 0; iter < max_iter; iter++) {
    oldlog = loglik;
    loglik = 0;

    // set the accumulating values to zeros and compute the inverse matrices and determinant constants
    TR.zeros();
    for (size_t i = 0; i < M; i++) {
      ME[i].zeros();
      CO[i].zeros();
      INV_CO[i] = inv(guessCO[i]);
      DET[i] = pow(2.0 * math::PI, -N / 2.0) * pow(det(guessCO[i]), -0.5);
    }
    sumState.zeros();

    // for each sequence, we will use forward-backward procedure and then accumulate
    for (size_t idx = 0; idx < seqs.size(); idx++) {
      // first calculate the emission probabilities of the sequence
      L = seqs[idx].n_cols;
      for (size_t t = 0; t < L; t++) {
	arma::vec e = seqs[idx].unsafe_col(t);
	for (size_t i = 0; i < M; i++)
	  emis_prob(i, t) = NORMAL_DENSITY(e, guessME[i], INV_CO[i], DET[i]);
      }

      loglik += GaussianHMM::Decode(L, guessTR, emis_prob, ps, fs, bs, s); // forward - backward procedure

      // accumulate expected transition & mean & covariance
      for (size_t t = 0; t < L - 1; t++) {
	for (size_t i = 0; i < M; i++) {
	  for (size_t j = 0; j < M; j++) {
	    TR(i, j) += fs(i, t) * guessTR(i, j) * emis_prob(j, t + 1) * bs(j, t + 1) / s[t + 1];
          }
        }
      }

      for (size_t t = 0; t < L; t++) {
	arma::vec e = seqs[idx].unsafe_col(t);
	for (size_t i = 0; i < M; i++) {
	  sumState[i] += ps(i, t);
          ME[i] += ps(i, t) * e;

	  arma::mat D = e;
          CO[i] += ps(i, t) * (D * trans(D));
	}
      }
      // end accumulate
    }

    // after accumulate all sequences: re-estimate transition & mean & covariance for the next iteration
    for (size_t i = 0; i < M; i++) {
      double s = 0;
      for (size_t j = 0; j < M; j++)
        s += TR(i, j);

      if (s == 0) {
	for (size_t j = 0; j < M; j++)
          guessTR(i, j) = 0;

	guessTR(i, i) = 1;
      }
      else {
	for (size_t j = 0; j < M; j++)
          guessTR(i, j) = TR(i, j) / s;
      }

      if (sumState[i] != 0) {
        guessME[i] = ME[i] / sumState[i];
	arma::mat D = guessME[i];
        CO[i] /= sumState[i];
        CO[i] -= D * trans(D);
	guessCO[i] = CO[i];
      }
    }
    // end re-estimate

    printf("Iter = %zu Loglik = %8.4f\n", iter, loglik);
    if (fabs(oldlog - loglik) < tol) {
      printf("\nConverged after %zu iterations\n", iter);
      break;
    }
    oldlog = loglik;
  }
}
