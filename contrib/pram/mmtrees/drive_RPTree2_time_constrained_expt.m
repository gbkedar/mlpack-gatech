function [ann_c] = drive_RPTree2_time_constrained_expt(c_vals, ...
						  Q, R, rank_mat, ...
						  output_file)

% Input variables:
% c_vals: The different values of c to build different RP trees
% Q: The query set
% R: The reference set
% rank_mat: The matrix containing the relative ranks of the
% reference points with respect to the queries (used to compute the
% error)
% flag: Whether the split-by-distance in the RPTrees are done on
% the original data set or the projected data set.
%
% Output:
% ann_c: A cell containing the results for each tree
% corresponding to a value of c. For each c, the result will be an
% array with avg. DC, avg. MC, avg. rank error and max. rank error

  [nDim, nPoints] = size(R);
  for i = 1:length(c_vals)
    
    % form a set of random projections
    P = random_unit_vector(nDim, 10);
    
    % build the tree for this c value
    display(sprintf('Building tree for %0.2f....', c_vals(i))); 
    T = RPTree_build2(R, P, c_vals(i));
    display(sprintf('Running the search algorithm...'))
    ann_c(i).a = RPTree_time_constrained_search(Q, R, T, rank_mat);
    ann_c(i).c = c_vals(i);
    clear T;

    clear P;
    
    
  end
  
  save(output_file, 'ann_c');
  