function [D, S] = ExpFamSparseCoding(type, X, k, lambda, max_iterations, ...
				     warm_start, D_initial)
%function [D, S] = ExpFamSparseCoding(type, X, k, lambda, max_iterations, ...
%				     warm_start, D_initial)
%
% Given: X - a matrix where each column is a document, and each row
% corresponds to vocabulary word. X_{i,j} is the number of times
% word i occurs in document j
% Good news: warm start is faster
% Bad news:  warm start yields poorer solutions than starting at 0

if nargin < 6
  warm_start = false;
end

if nargin < 7
  D_initial = [];
end

if type == 'p'
  ComputeFullObjective = ...
      @(D, S, X, lambda) ...
      ComputePoissonFullObjective(D, S, X, lambda);
  
elseif type == 'b'
  ComputeFullObjective = ...
      @(D, S, X, lambda) ...
      ComputeBernoulliFullObjective(D, S, X, lambda);

elseif type == 'g'
  ComputeFullObjective = ...
      @(D, S, X, lambda) ...
      ComputeGaussianFullObjective(D, S, X, lambda);

end


[d n] = size(X);

% hardcoded for now
alpha = 1e-4;
beta = 0.9;

c = 1; %100;

% Set Initial Dictionary
if isempty(D_initial)
  D = c * normcols(normrnd(0,1,d,k));
else
  D = D_initial;
end

% Sparse codes update via feature-sign
fprintf('INITIAL SPARSE CODING STEP\n');
S = UpdateSparseCodes(type, X, D, lambda, [], alpha, beta);
fprintf('norm(S) = %f\t||S||_1 = %f\t%f%% sparsity\n', ...
	norm(S), sum(sum(abs(S))), (nnz(S) / prod(size(S))) * 100);
%fprintf('DONE SPARSE CODING\n');
%pause;

fprintf('\t\t\tObjective value: %f\n', ...
	ComputeFullObjective(D, S, X, lambda));


converged = false;
iteration_num = 0;

if iteration_num == max_iterations
  converged = true;
end

while ~converged
  iteration_num = iteration_num + 1;
  fprintf('PSC Iteration %d\n', iteration_num);
  
  % Dictionary update
  fprintf('DICTIONARY LEARNING STEP\n');
  D = DictionaryProjectedGradient(type, D, S, X, alpha, beta, 2);
  %fprintf('DONE LEARNING DICTIONARY\n');
  %pause;

  fprintf('\t\t\tObjective value: %f\n', ...
	  ComputeFullObjective(D, S, X, lambda));
    

  % Sparse codes update via feature-sign
  fprintf('SPARSE CODING STEP\n');
  if warm_start
    S = UpdateSparseCodes(type, X, D, lambda, S, alpha, beta);
  else
    S = UpdateSparseCodes(type, X, D, lambda, [], alpha, beta);
  end
  fprintf('norm(S) = %f\t||S||_1 = %f\t%f%% sparsity\n', ...
	  norm(S), sum(sum(abs(S))), (nnz(S) / prod(size(S))) * 100);
  %fprintf('DONE SPARSE CODING\n');
  %pause;

  fprintf('\t\t\tObjective value: %f\n', ...
	  ComputeFullObjective(D, S, X, lambda));
  
  
  % check convergence criterion - temporarily just 10 iterations
  % for debugging
  if iteration_num == max_iterations
    converged = true;
  end
end

fprintf('Done learning\n');
