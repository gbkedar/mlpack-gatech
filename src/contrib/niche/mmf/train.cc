/**
 * @file train.cc
 *
 * This file contains the program to estimate Hidden Markov Model parameter
 * using training sequences.
 *
 * It use two algorithm: Baum-Welch (EM) and Viterbi
 *
 * Usage:
 *   train --type=TYPE --profile=PROFILE --seqfile=FILE [OPTIONS]
 * See the usage() function for complete option list
 */

#include "fastlib/fastlib.h"
#include "support.h"
#include "discreteHMM.h"
#include "gaussianHMM.h"
#include "mixgaussHMM.h"
#include "mixtureDST.h"

using namespace hmm_support;

bool train_baumwelch();
bool train_viterbi();
void usage();

const fx_entry_doc hmm_train_main_entries[] = {
  {"type", FX_REQUIRED, FX_STR, NULL,
   "  HMM type : discrete | gaussian | mixture.\n"},
  {"algorithm", FX_PARAM, FX_STR, NULL,
   "  Training algoritm: baumwelch | viterbi.\n"},
  {"seqfile", FX_REQUIRED, FX_STR, NULL,
   "  Output file for the data sequences.\n"},
  {"guess", FX_PARAM, FX_STR, NULL,
   "  File containing guessing HMM model profile.\n"},
  {"numstate", FX_PARAM, FX_INT, NULL,
   "  If no guessing profile specified, at least provide the number of states.\n"},
  {"profile", FX_REQUIRED, FX_STR, NULL,
   "  Output file containing trained HMM profile.\n"},
  {"maxiter", FX_PARAM, FX_INT, NULL,
   "  Maximum number of iterations, default = 500.\n"},
  {"tolerance", FX_PARAM, FX_DOUBLE, NULL,
   "  Error tolerance on log-likelihood as a stopping criteria.\n"},
  FX_ENTRY_DOC_DONE
};

const fx_submodule_doc hmm_train_main_submodules[] = {
  FX_SUBMODULE_DOC_DONE
};

const fx_module_doc hmm_train_main_doc = {
  hmm_train_main_entries, hmm_train_main_submodules,
  "This is a program training HMM models from data sequences. \n"
};

void usage() {
  printf("\nUsage:\n"
	 "  train --type=={discrete|gaussian|mixture} OPTION\n"
	 "[OPTIONS]\n"
	 "  --algorithm={baumwelch|viterbi} : algorithm used for training, default Baum-Welch\n"
	 "  --seqfile=file   : file contains input sequences\n"
	 "  --guess=file     : file contains guess HMM profile\n"
	 "  --numstate=NUM   : if no guess profile is specified, at least specify the number of state\n"
	 "  --profile=file   : output file for estimated HMM profile\n"
	 "  --maxiter=NUM    : maximum number of iteration, default=500\n"
	 "  --tolerance=NUM  : error tolerance on log-likelihood, default=1e-3\n"
	 );
}

int main(int argc, char* argv[]) {
  fx_init(argc, argv, &hmm_train_main_doc);
  bool s = true;
  if (fx_param_exists(NULL,"type")) {
    const char* algorithm = fx_param_str(NULL, "algorithm", "baumwelch");
    if (strcmp(algorithm,"baumwelch")==0)
      s = train_baumwelch();
    else if (strcmp(algorithm,"viterbi")==0)
      s = train_viterbi();
    else {
      printf("Unrecognized algorithm: must be baumwelch or viterbi !!!\n");
      s = false;
    }
  }
  else {
    printf("Unrecognized type: must be: discrete | gaussian | mixture  !!!\n");
    s = false;
  }
  if (!(s)) usage();
  fx_done(NULL);
}

bool train_baumwelch_discrete();
bool train_baumwelch_gaussian();
bool train_baumwelch_mixture();

bool train_baumwelch() {
  const char* type = fx_param_str_req(NULL, "type");
  if (strcmp(type, "discrete")==0)
    return train_baumwelch_discrete();
  else if (strcmp(type, "gaussian")==0)
    return train_baumwelch_gaussian();
  else if (strcmp(type, "mixture")==0)
    return train_baumwelch_mixture();
  else {
    printf("Unrecognized type: must be: discrete | gaussian | mixture !!!\n");
    return false;
  }
}

bool train_viterbi_discrete();
bool train_viterbi_gaussian();
bool train_viterbi_mixture();

bool train_viterbi() {
  const char* type = fx_param_str_req(NULL, "type");
  if (strcmp(type, "discrete")==0)
    return train_viterbi_discrete();
  else if (strcmp(type, "gaussian")==0)
    return train_viterbi_gaussian();
  else if (strcmp(type, "mixture")==0)
    return train_viterbi_mixture();
  else {
    printf("Unrecognized type: must be: discrete | gaussian | mixture !!!\n");
    return false;
  }
}

bool train_baumwelch_mixture() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }

  MixtureofGaussianHMM hmm;
  ArrayList<Matrix> seqs;

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.mix.out");

  load_matrix_list(seqin, &seqs);

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else {
    hmm.Init();
    printf("Automatic initialization not supported !!!");
    return false;
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  hmm.TrainBaumWelch(seqs, maxiter, tol);

  hmm.SaveProfile(proout);

  return true;
}

bool train_baumwelch_gaussian() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }
  GaussianHMM hmm;
  ArrayList<Matrix> seqs;

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.gauss.out");

  load_matrix_list(seqin, &seqs);

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else { // otherwise initialized using information from the data
    int numstate = fx_param_int_req(NULL, "numstate");
    printf("Generate HMM parameters: NUMSTATE = %d\n", numstate);
    hmm.InitFromData(seqs, numstate);
    printf("Done.\n");
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  printf("Training ...\n");
  hmm.TrainBaumWelch(seqs, maxiter, tol);
  printf("Done.\n");

  hmm.SaveProfile(proout);

  return true;
}

bool train_baumwelch_discrete() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.dis.out");

  ArrayList<Vector> seqs;
  load_vector_list(seqin, &seqs);

  DiscreteHMM hmm;

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load HMM parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else { // otherwise randomly initialized using information from the data
    int numstate = fx_param_int_req(NULL, "numstate");
    printf("Randomly generate parameters: NUMSTATE = %d\n", numstate);
    hmm.InitFromData(seqs, numstate);
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  hmm.TrainBaumWelch(seqs, maxiter, tol);

  hmm.SaveProfile(proout);

  return true;
}

bool train_viterbi_mixture() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }
  
  MixtureofGaussianHMM hmm;
  ArrayList<Matrix> seqs;

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.mix.out");

  load_matrix_list(seqin, &seqs);

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else {
    hmm.Init();
    printf("Automatic initialization not supported !!!");
    return false;
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  hmm.TrainViterbi(seqs, maxiter, tol);

  hmm.SaveProfile(proout);

  return true;
}

bool train_viterbi_gaussian() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }
  
  GaussianHMM hmm;
  ArrayList<Matrix> seqs;

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.gauss.viterbi.out");

  load_matrix_list(seqin, &seqs);

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else { // otherwise initialized using information from the data
    int numstate = fx_param_int_req(NULL, "numstate");
    printf("Generate parameters: NUMSTATE = %d\n", numstate);
    hmm.InitFromData(seqs, numstate);
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  hmm.TrainViterbi(seqs, maxiter, tol);

  hmm.SaveProfile(proout);

  return true;
}

bool train_viterbi_discrete() {
  if (!fx_param_exists(NULL, "seqfile")) {
    printf("--seqfile must be defined.\n");
    return false;
  }

  DiscreteHMM hmm;
  ArrayList<Vector> seqs;

  const char* seqin = fx_param_str_req(NULL, "seqfile");
  const char* proout = fx_param_str(NULL, "profile", "pro.dis.viterbi.out");

  load_vector_list(seqin, &seqs);

  if (fx_param_exists(NULL, "guess")) { // guessed parameters in a file
    ArrayList<Matrix> matlst;
    const char* guess = fx_param_str_req(NULL, "guess");
    printf("Load parameters from file %s\n", guess);
    hmm.InitFromFile(guess);
  }
  else { // otherwise randomly initialized using information from the data
    int numstate = fx_param_int_req(NULL, "numstate");
    printf("Generate parameters with NUMSTATE = %d\n", numstate);
    hmm.InitFromData(seqs, numstate);
  }

  int maxiter = fx_param_int(NULL, "maxiter", 500);
  double tol = fx_param_double(NULL, "tolerance", 1e-3);

  hmm.TrainViterbi(seqs, maxiter, tol);

  hmm.SaveProfile(proout);

  return true;
}
