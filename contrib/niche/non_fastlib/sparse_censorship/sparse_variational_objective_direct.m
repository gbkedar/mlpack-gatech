function [f,g,h] = sparse_variational_objective_direct(a, laplace_var, exp_param)

fprintf('f(%f) = ', a);

%b = (1 + sqrt(1 + 8 * exp_param * laplace_var^2 * a / (a - 1))) ...
%    / (4 * exp_param * a);

gamma_a = gamma(a);

t = sqrt(1 + (8 * a * exp_param * laplace_var^2) / (a - 1));
eel2 = 8 * exp_param * laplace_var^2;

f = -(2 * a - t + log((1 + t) / (4 * a * exp_param)) ...
      + 2 * log(gamma_a) + (1 - 2 * a) * psi(a)) / 2;

fprintf('%f\n', f);

g = -(1 - 6 * a + 4 * a^2 + t - 2 * (a - 1) * a * (2 * a - 1) * psi(1, a)) ...
    / (4 * (a - 1) * a);

h = (1 + t - a * (2 * a^2 * (1 + eel2) + 3 * (1 + t) ...
		  + 0.5 * eel2 * (2 + t) ...
		  - 2 * a * (2 + t + eel2 * (1 + t))) ...
     + 2 * (a - 1)^2 * a^2 * (a - 1 + a * eel2) ...
     * (2 * psi(1,a) + (2 * a - 1) * psi(2, a))) ...
    / (4 * (a - 1)^2 * a^2 * (a - 1 + a * eel2));
