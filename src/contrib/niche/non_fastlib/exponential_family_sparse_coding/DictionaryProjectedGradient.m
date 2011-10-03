function D_opt = DictionaryProjectedGradient(type, D_0, S, T, alpha, beta, verbose)
%function D_opt = DictionaryProjectedGradient(type, D_0, S, T, alpha, beta, verbose)
%
% T are the sufficient statistics for the data, stored as
% num_features by num_points
%
% Nocedal and Wright suggested default alpha = 1e-4,
% but Boyd and Vandenberghe suggest using 0.01 to 0.3
%
% We use Armijo line search, with projection operator P, gradient g, and
% termination condition
%   f(P(x + t*g)) >= f(x) + alpha * g^T (P(x + t*g) - x)


if nargin < 5
  alpha = 1e-4;
end
if nargin < 6
  beta = 0.9;
end
if nargin < 7
  verbose = false;
end

%obj_tol = 1e-6;
obj_tol = 1e-1;

% bound on norm of atoms
c = 1; %100;


if type == 'p'
  ComputeDictionaryObjective = ...
      @(D, S, T) ...
      ComputePoissonDictionaryObjective(D, S, T);

  ComputeDictionaryGradient = ...
      @(D, S, T) ...
      ComputePoissonDictionaryGradient(D, S, T);

elseif type == 'b'
  ComputeDictionaryObjective = ...
      @(D, S, T) ...
      ComputeBernoulliDictionaryObjective(D, S, T);

  ComputeDictionaryGradient = ...
      @(D, S, T) ...
      ComputeBernoulliDictionaryGradient(D, S, T);
  
elseif type == 'g'
  ComputeDictionaryObjective = ...
      @(D, S, T) ...
      ComputeGaussianDictionaryObjective(D, S, T);

  ComputeDictionaryGradient = ...
      @(D, S, T) ...
      ComputeGaussianDictionaryGradient(D, S, T);
  
end

[d k] = size(D_0);
n = size(S, 2);


for main_iteration = 1:1000

  if verbose
    fprintf('Main Iteration %d\n', main_iteration);
  end

  % compute gradient
  %grad = zeros(size(D_0));
  %grad = grad - T * S';
  % easy to understand version of the above line
  %for i = 1:n
  %  grad = grad - T(:,i) * S(:,i)';
  %end

  %grad = grad + exp(D_0 * S) * S';
  % easy to understand version of the above line
  %for i = 1:n
  %  grad = grad + exp(D_0 * S(:,i)) * S(:,i)';
  %end

  grad = ComputeDictionaryGradient(D_0, S, T);
  
  % currently, we are NOT doing ANY NORMALIZATION
  %grad = grad / n; % not sure how well this works, maybe we should switch to the below again
  %grad = grad / sqrt(trace(grad' * grad)); % seems to work well
  
  % do line search along direction of negative gradient, using projected evaluation to find D_opt
  
  %start with step size t = 1, decreasing by beta until Armijo condition is satisfied
  %   f(P(x + t*g)) >= f(x) + alpha * g^T (P(x + t*g) - x)
  
  f_0 = ComputeDictionaryObjective(D_0, S, T);
  
  if verbose == 2
    fprintf('\t\t\t\ttrace(grad^T * grad) = %f\n', trace(grad' * grad));
  end
    
  

  fro_norm_grad = norm(grad, 'fro');
  if fro_norm_grad > 2 * c * sqrt(k)
    % if t is any larger than this, then the resulting D_t would never be feasible
    t = sqrt(k) * c / fro_norm_grad; 
  else
    t = 1;
  end
  
  iteration_num = 0;
  done = false;
  prev_best_f = f_0;
  while ~done
    iteration_num = iteration_num + 1;
    if verbose == 2
      fprintf('Iteration %d, t = %f\n', iteration_num, t);
    end

    D_t = D_0 - t * grad;
    norms = sqrt(sum(D_t .^ 2));
    % project if necessary
    for i = find((norms > c))
      D_t(:,i) = c * D_t(:,i) / norms(i);
    end
    
    f_t = ComputeDictionaryObjective(D_t, S, T);
    if verbose == 2
      fprintf('f_0 = %f\tf_t = %f\n', f_0, f_t);
      fprintf('\t\t\t\ttrace(grad^T * (D_t - D_0)) = %f\n', trace(grad' * (D_t - D_0))); 
      fprintf('\t\t\t\tnorm(D_t - D_0) = %f\n', norm(D_t - D_0));
    end

    if f_t <= f_0 + alpha * trace(grad' * (D_t - D_0))
      done = true;
      if verbose
	fprintf('\t\tCompleted %d line search iterations\n', iteration_num);
	fprintf('\t\tObjective value: %f\n', f_t);
      end
      if f_t > prev_best_f
	error('Objective increased! Aborting...');
	return;
      end
      if prev_best_f - f_t < obj_tol
	if verbose
	  fprintf('Improvement to objective below tolerance. Finished.\n');
	end
	D_opt = D_t;
	return;
      end
      prev_best_f = f_t;
    end
    %pause;
    t = beta * t;
  end

  D_0 = D_t;
end

D_opt = D_0;
