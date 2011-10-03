/**
 * @author Parikshit Ram (pram@cc.gatech.edu)
 * @file mog_em.cc
 *
 * Implementation for the loglikelihood function, the EM algorithm
 * and also computes the K-means for getting an initial point
 *
 */

#include "mog_em.h"
#include "phi.h"
#include "math_functions.h"

void MoGEM::ExpectationMaximization(Matrix& data_points) {

  // Declaration of the variables */
  size_t num_points;
  size_t dim, num_gauss;
  double sum, tmp;
  ArrayList<Vector> mu_temp, mu;
  ArrayList<Matrix> sigma_temp, sigma;
  Vector omega_temp, omega, x;
  Matrix cond_prob;
  long double l, l_old, best_l, INFTY = 99999, TINY = 1.0e-10;

  // Initializing values
  dim = dimension();
  num_gauss = number_of_gaussians();
  num_points = data_points.n_cols();

  // Initializing the number of the vectors and matrices
  // according to the parameters input
  mu_temp.Init(num_gauss);
  mu.Init(num_gauss);
  sigma_temp.Init(num_gauss);
  sigma.Init(num_gauss);
  omega_temp.Init(num_gauss);
  omega.Init(num_gauss);

  // Allocating size to the vectors and matrices
  // according to the dimensionality of the data
  for(size_t i = 0; i < num_gauss; i++) {
    mu_temp[i].Init(dim);
    mu[i].Init(dim);
    sigma_temp[i].Init(dim, dim);
    sigma[i].Init(dim, dim);
  }
  x.Init(dim);
  cond_prob.Init(num_gauss, num_points);

  best_l = -INFTY;
  size_t restarts = 0;
  // performing 5 restarts and choosing the best from them
  while (restarts < 5) {

    // assign initial values to 'mu', 'sig' and 'omega' using k-means
    KMeans(data_points, &mu_temp, &sigma_temp, &omega_temp, num_gauss);

    l_old = -INFTY;

    // calculates the loglikelihood value
    l = Loglikelihood(data_points, mu_temp, sigma_temp, omega_temp);

    // added a check here to see if any
    // significant change is being made
    // at every iteration
    while (l - l_old > TINY) {
      // calculating the conditional probabilities
      // of choosing a particular gaussian given
      // the data and the present theta value
      for (size_t j = 0; j < num_points; j++) {
	x.CopyValues(data_points.GetColumnPtr(j));
	sum = 0;
	for (size_t i = 0; i < num_gauss; i++) {
	  tmp = phi(x, mu_temp[i], sigma_temp[i]) * omega_temp.get(i);
	  cond_prob.set(i, j, tmp);
	  sum += tmp;
	}
	for (size_t i = 0; i < num_gauss; i++) {
	  tmp = cond_prob.get(i, j);
	  cond_prob.set(i, j, tmp / sum);
	}
      }

      // calculating the new value of the mu
      // using the updated conditional probabilities
      for (size_t i = 0; i < num_gauss; i++) {
	sum = 0;
	mu_temp[i].SetZero();
	for (size_t j = 0; j < num_points; j++) {
	  x.CopyValues(data_points.GetColumnPtr(j));
	  la::AddExpert(cond_prob.get(i, j), x, &mu_temp[i]);
	  sum += cond_prob.get(i, j);
	}
	la::Scale((1.0 / sum), &mu_temp[i]);
      }

      // calculating the new value of the sig
      // using the updated conditional probabilities
      // and the updated mu
      for (size_t i = 0; i < num_gauss; i++) {
	sum = 0;
	sigma_temp[i].SetZero();
	for (size_t j = 0; j < num_points; j++) {
	  Matrix co, ro, c;
	  c.Init(dim, dim);
	  x.CopyValues(data_points.GetColumnPtr(j));
	  la::SubFrom(mu_temp[i] , &x);
	  co.AliasColVector(x);
	  ro.AliasRowVector(x);
	  la::MulOverwrite(co, ro, &c);
	  la::AddExpert(cond_prob.get(i, j), c, &sigma_temp[i]);
	  sum += cond_prob.get(i, j);
	}
	la::Scale((1.0 / sum), &sigma_temp[i]);
      }

      // calculating the new values for omega
      // using the updated conditional probabilities
      Vector identity_vector;
      identity_vector.Init(num_points);
      identity_vector.SetAll(1.0 / num_points);
      la::MulOverwrite(cond_prob, identity_vector, &omega_temp);

      l_old = l;
      l = Loglikelihood(data_points, mu_temp, sigma_temp, omega_temp);
    }

    // putting a check to see if the best one is chosen
    if(l > best_l){
      best_l = l;
      for (size_t i = 0; i < num_gauss; i++) {
	mu[i].CopyValues(mu_temp[i]);
	sigma[i].CopyValues(sigma_temp[i]);
      }
      omega.CopyValues(omega_temp);
    }
    restarts++;
  }

  for (size_t i = 0; i < num_gauss; i++) {
    set_mu(i, mu[i]);
    set_sigma(i, sigma[i]);
  }
  set_omega(omega);

  NOTIFY("loglikelihood value of the estimated model: %Lf\n", best_l);
  return;
}

long double MoGEM::Loglikelihood(Matrix& data_points, ArrayList<Vector>& means,
			       ArrayList<Matrix>& covars, Vector& weights) {

  size_t i, j;
  Vector x;
  long double likelihood, loglikelihood = 0;

  x.Init(data_points.n_rows());

  for (j = 0; j < data_points.n_cols(); j++) {
    x.CopyValues(data_points.GetColumnPtr(j));
    likelihood = 0;
    for(i = 0; i < number_of_gaussians() ; i++){
      likelihood += weights.get(i) * phi(x, means[i], covars[i]);
    }
    loglikelihood += log(likelihood);
  }
  return loglikelihood;
}

void MoGEM::KMeans(Matrix& data, ArrayList<Vector> *means,
		 ArrayList<Matrix> *covars, Vector *weights, size_t value_of_k){


  ArrayList<Vector> mu, mu_old;
  double* tmpssq;
  double* sig;
  double* sig_best;
  size_t *y;
  Vector x, diff;
  Matrix ssq;
  size_t i, j, k, n, t, dim;
  double score, score_old, sum;

  n = data.n_cols();
  dim = data.n_rows();
  mu.Init(value_of_k);
  mu_old.Init(value_of_k);
  tmpssq = (double*)malloc(value_of_k * sizeof( double ));
  sig = (double*)malloc(value_of_k * sizeof( double ));
  sig_best = (double*)malloc(value_of_k * sizeof( double ));
  ssq.Init(n, value_of_k);

  for( i = 0; i < value_of_k; i++){
    mu[i].Init(dim);
    mu_old[i].Init(dim);
  }
  x.Init(dim);
  y = (size_t*)malloc(n * sizeof(size_t));
  diff.Init(dim);

  score_old = 999999;

  // putting 5 random restarts to obtain the k-means
  for(i = 0; i < 5; i++){
    t = -1;
    for (k = 0; k < value_of_k; k++){
      t = (t + 1 + (rand()%((n - 1 - (value_of_k - k)) - (t + 1))));
      mu[k].CopyValues(data.GetColumnPtr(t));
      for(j = 0; j < n; j++){
	x.CopyValues( data.GetColumnPtr(j));
	la::SubOverwrite(mu[k], x, &diff);
	ssq.set( j, k, la::Dot(diff, diff));
      }
    }
    min_element(ssq, y);

    do{
      for(k = 0; k < value_of_k; k++){
	mu_old[k].CopyValues(mu[k]);
      }

      for(k = 0; k < value_of_k; k++){
	size_t p = 0;
	mu[k].SetZero();
	for(j = 0; j < n; j++){
	  x.CopyValues(data.GetColumnPtr(j));
	  if(y[j] == k){
	    la::AddTo(x, &mu[k]);
	    p++;
	  }
	}

	if(p == 0){
	}
	else{
	  double sc = 1 ;
	  sc = sc / p;
	  la::Scale(sc , &mu[k]);
	}
	for(j = 0; j < n; j++){
	  x.CopyValues(data.GetColumnPtr(j));
	  la::SubOverwrite(mu[k], x, &diff);
	  ssq.set(j, k, la::Dot(diff, diff));
	}
      }
      min_element(ssq, y);

      sum = 0;
      for(k = 0; k < value_of_k; k++) {
	la::SubOverwrite(mu[k], mu_old[k], &diff);
	sum += la::Dot(diff, diff);
      }
    }while(sum != 0);

    for(k = 0; k < value_of_k; k++){
      size_t p = 0;
      tmpssq[k] = 0;
      for(j = 0; j < n; j++){
	if(y[j] == k){
	  tmpssq[k] += ssq.get(j, k);
	  p++;
	}
      }
      sig[k] = sqrt(tmpssq[k] / p);
    }

    score = 0;
    for(k = 0; k < value_of_k; k++){
      score += tmpssq[k];
    }
    score = score / n;

    if (score < score_old) {
      score_old = score;
      for(k = 0; k < value_of_k; k++){
	(*means)[k].CopyValues(mu[k]);
	sig_best[k] = sig[k];
      }
    }
  }

  for(k = 0; k < value_of_k; k++){
    x.SetAll(sig_best[k]);
    (*covars)[k].SetDiagonal(x);
  }
  double tmp = 1;
  (*weights).SetAll(tmp / value_of_k);
  return;
}
