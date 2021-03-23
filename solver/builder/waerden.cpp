#include "solver/builder/waerden.h"

#include <vector>

namespace solver {
namespace builder {

void Waerden(Solver &solver, int j, int k, int n) {
  std::vector<Var> x;
  for (int i = 1; i <= n; ++i) {
    x.push_back(solver.NewVar("x" + std::to_string(i)));
  }

  for (int d = 1; d < n; ++d) {
    // Disallow j d-spaced 0s at positions: i, i+d, ..., i+(j-1)d.
    for (int i = 1; i + (j - 1) * d <= n; ++i) {
      std::vector<Lit> clause;
      for (int a = 0; a < j; ++a) {
        clause.push_back(x[i + a * d - 1]);
      }
      solver.AddClause(clause);
    }
    // Disallow k d-spaced 1s at positions: i, i+d, ..., i+(j-1)d.
    for (int i = 1; i + (k - 1) * d <= n; ++i) {
      std::vector<Lit> clause;
      for (int a = 0; a < k; ++a) {
        clause.push_back(~x[i + a * d - 1]);
      }
      solver.AddClause(clause);
    }
  }
}

} // namespace builder
} // namespace solver
