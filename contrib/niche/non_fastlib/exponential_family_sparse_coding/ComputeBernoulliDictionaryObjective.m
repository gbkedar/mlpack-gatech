function f = ComputeBernoulliDictionaryObjective(D, S, T)
%function f = ComputeBernoulliDictionaryObjective(D, S, T)

n = size(S, 2);

f = -trace(D' * T * S') + sum(sum(log(1 + exp(D * S)))); % check this for numerical overflow

f = f / n; % does this help?
