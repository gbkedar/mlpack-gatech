function S = UpdateSparseCodes(type, T, D, lambda, S_initial, alpha, beta, verbose)
%function S = UpdateSparseCodes(type, T, D, lambda, S_initial, alpha, beta)
%
% T - sufficient statistics for the data
% D - dictionary of size n_dimensions by n_atoms
% lambda - l1-norm regularization parameter
% S_initial - initial guess for sparse codes of all points, size is n_atoms by n_points
% alpha - sufficient decrease parameter for Armijo rule
% beta - 0 < beta < 1 - specifies geometric rate of decrease of line search parameter


if nargin < 6
  alpha = 1e-4;
end
if nargin < 7
  beta = 0.9;
end
if nargin < 8
  verbose = false;
end


if type == 'p'
  ComputeSparseCodesObjective = ...
      @(D, s, Dt_t, lambda) ...
      ComputePoissonSparseCodesObjective(D, s, Dt_t, lambda);

  ComputeSparseCodesSubgradient = ...
      @(D, s, t, lambda) ...
      ComputePoissonSparseCodesSubgradient(D, s, t, lambda);

  ComputeTargetsAndRegressors = ...
      @(D, s, t) ...
      ComputePoissonTargetsAndRegressors(D, s, t);
  
elseif type == 'b'
  ComputeSparseCodesObjective = ...
      @(D, s, Dt_t, lambda) ...
      ComputeBernoulliSparseCodesObjective(D, s, Dt_t, lambda);
  
  ComputeSparseCodesSubgradient = ...
      @(D, s, t, lambda) ...
      ComputeBernoulliSparseCodesSubgradient(D, s, t, lambda);

  ComputeTargetsAndRegressors = ...
      @(D, s, t) ...
      ComputeBernoulliTargetsAndRegressors(D, s, t);
  
elseif type == 'g'
  ComputeSparseCodesObjective = ...
      @(D, s, Dt_t, lambda) ...
      ComputeGaussianSparseCodesObjective(D, s, Dt_t, lambda);
  
  ComputeSparseCodesSubgradient = ...
      @(D, s, t, lambda) ...
      ComputeGaussianSparseCodesSubgradient(D, s, t, lambda);

  ComputeTargetsAndRegressors = ...
      @(D, s, t) ...
      ComputeGaussianTargetsAndRegressors(D, s, t);
  
end  


obj_tol = 1e-6; % hardcoded for now

[d k] = size(D);

n = size(T, 2);

S = zeros(k, n);

rank_AtA = rank(D); % useful precomputation for later

% something may be wrong with the parfor version; need to double check that badness isn't happening
parfor i = 1:n
%for i = 1:n

  if mod(i, 100) == 0
    fprintf('%d ', i);
  end

  if ~isempty(S_initial)
    s_0 = S_initial(:,i);
  else
    s_0 = zeros(k, 1);
  end
  
  % precomputation, useful for later
  Dt_T_i = D' * T(:,i);

  if verbose
    fprintf('Point %d\tStarting Objective value: %f\n', ...
	    i, ComputeSparseCodesObjective(D, s_0, Dt_T_i, lambda));
  end
  
  max_main_iterations = 10;
  main_iteration = 0;
  main_converged = false;
  while (main_iteration <= max_main_iterations) && ~main_converged

    main_iteration = main_iteration + 1;
    if verbose
      fprintf('Main Iteration %d\n', main_iteration);
    end
    
    %Lambda = ComputePoissonLambda(D, s_0);
    %Lambda = exp(D * s_0);
    %z = ComputePoissonZ(D, s_0, T(:,i), Lambda);
    %z = (T(:,i) ./ Lambda) - ones(d, 1) + D * s_0;
    %regressors = bsxfun(@times, sqrt(Lambda), D);
    %targets = sqrt(Lambda) .* z;

    [targets, regressors] = ...
	ComputeTargetsAndRegressors(D, s_0, T(:,i));
    
    
    AtA = regressors' * regressors;
    %regressors
    %targets
    
    %fprintf('starting goodness: %f\n', norm(regressors * s_0 - targets));

    % lambda or lambda / 2 ? I think lambda
    s_1 = l1ls_featuresign(regressors, targets, lambda, s_0, AtA, ...
			   rank_AtA);
    %s_0
    %s_1
    
    
    f_0 = ComputeSparseCodesObjective(D, s_0, Dt_T_i, lambda);
    
    
    
    % choose a subgradient at s_0
    subgrad = ComputeSparseCodesSubgradient(D, s_0, T(:,i), lambda);
    %subgrad = -D' * T(:,i);
    %subgrad = subgrad + D' * exp(D * s_0);
    %subgrad = subgrad + lambda * ((s_0 > 0) - (s_0 < 0)); % handle possibly non-differentiable component by using subgradient
    
    
    % Armijo'ish line search to select next s
    t = 1;
    iteration_num = 0;
    done = false;
    prev_best_f = f_0;
    while ~done
      iteration_num = iteration_num + 1;

      
      s_t = t * s_1 + (1 - t) * s_0;
      
      f_t = ComputeSparseCodesObjective(D, s_t, Dt_T_i, lambda);

      if iteration_num == 0 && f_t > f_0
	fprintf('main_iteration = %d\n', main_iteration);
	fprintf('sc inner iteration %d\n', iteration_num);
	fprintf('f_0 = %e\tf_t = %e\n', f_0, f_t);
      end


      
      if f_t <= f_0 + alpha * subgrad' * (s_t - s_0)
	done = true;
	if verbose
	  fprintf('\t\tCompleted %d line search iterations\n', iteration_num);
	  fprintf('\t\tObjective value: %f\n', f_t);
	end
	if iteration_num > 1
	  fprintf('Done: %d line search iterations\n', iteration_num);
	end
	
	if f_t > prev_best_f
	  error('Objective increased! Aborting...');
	  %return;
	end
	if prev_best_f - f_t < obj_tol
	  if verbose
	    fprintf('Improvement to objective below tolerance. Finished.\n');
	  end
	  main_converged = true;
	end
	prev_best_f = f_t;
      end
      t = beta * t;
    end
    s_0 = s_t;
  end
  
  S(:,i) = s_0;
  
end
