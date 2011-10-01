#ifndef __GRAPH_H
#define __GRAPH_H
#include "fastlib/fastlib.h"
#include <queue>

class Graph {
  typedef GenMatrix<bool> AdjacentMatrix;
  AdjacentMatrix adjacent;
  Matrix weight;
 public:
  bool isEdge(size_t i, size_t j) const { return adjacent.get(i, j); }
  double getW(size_t i, size_t j) const { return weight.get(i, j); }
  const Matrix& getW() const { return weight; }

  bool& refEdge(size_t i, size_t j) { return adjacent.ref(i, j); }
  double& refW(size_t i, size_t j) { return weight.ref(i, j); }

  size_t n_nodes() const { return adjacent.n_rows(); }

  void Init(size_t n) {
    adjacent.Init(n, n);
    weight.Init(n, n);
  }

  void InitFromFile(const char* f, double threshold = 0);
  void ThresholdEdges(double threshold);
};

typedef ArrayList<size_t> Path;

// Need isEdge() & n_nodes() functions
template <class Graph>
void BreadthFirstSearch(size_t s, size_t t, const Graph& g, Path* p) {
  std::queue<size_t> q;
  GenVector<bool> visited;
  GenVector<size_t> previous;

  visited.Init(g.n_nodes());
  previous.Init(g.n_nodes());

  visited.SetAll(false);
  previous.SetAll(-1);

  q.push(t); visited[t] = true;
  while (!q.empty() && !visited[s]) {
    int v = q.front(); q.pop();
    for (size_t u = 0; u < g.n_nodes(); u++)
      if (g.isEdge(u, v) && !visited[u]) {
	      q.push(u); 
	      previous[u] = v;
	      visited[u] = true;
      }
  }
  p->Init();
  if (!visited[s]) return;
  p->PushBackCopy(s);
  while (s != t) {
    s = previous[s];
    p->PushBackCopy(s);
  }
}

template <class Graph>
class MaxFlowAugmentedGraph {
  const Graph& g;
  Matrix c;
  Matrix f;
 public:
  MaxFlowAugmentedGraph(const Graph& g_, const Matrix& c_, Matrix& f_)
    : g(g_) {
    c.Alias(c_);
    f.Alias(f_);
  }
  bool isEdge(size_t i, size_t j) const {
    return (g.isEdge(i, j) && f.get(i, j) < c.get(i, j)) ||
      (g.isEdge(j, i) && f.get(j, i) > 0);
  }
  
  size_t n_nodes() const { return g.n_nodes(); }
  
  void ComputeMaxFlow(size_t s, size_t t) {
    while (1) {
      Path p;
      BreadthFirstSearch(s, t, *this, &p);
      //ot::Print(p);
      if (p.size() == 0) break;
      double val = CalAugmentValue(p);
      AugmentPath(p, val);
    }
  }
 private:
  double CalAugmentValue(const Path& p) {
    printf("Augment path\n");
    double augmentValue = INFINITY;
    for (size_t k = 0; k < p.size()-1; k++) {
      size_t i = p[k], j = p[k+1];
      double val;
      if (g.isEdge(i, j) && f.get(i, j) < c.get(i, j)) // forward
        val = c.get(i, j) - f.get(i, j);
      else // backward
        val = f.get(j, i);
      printf("%d %d --> %f\n", i, j, val);
      if (val < augmentValue) augmentValue = val;
    }
    return augmentValue;
  }
  
  void AugmentPath(const Path& p, double val) {
    for (size_t k = 0; k < p.size()-1; k++) {
      size_t i = p[k], j = p[k+1];
      if (g.isEdge(i, j) && f.get(i, j) < c.get(i, j)) // forward
        f.ref(i, j) += val;
      else // backward
        f.ref(j, i) -= val;
    }
  }
};

// Max flow from a correctly initialized flow (e.g. the zero flow)
// Need isEdge() and n_nodes() function for class Graph
template <class Graph>
void MaxFlow(size_t s, size_t t, const Graph& g, 
  const Matrix& c, Matrix* f) {
  MaxFlowAugmentedGraph<Graph> ag(g, c, *f);
  ag.ComputeMaxFlow(s, t);
}

#endif
