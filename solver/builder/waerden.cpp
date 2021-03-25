#include "solver/builder/waerden.h"

#include <vector>

#include "solver/builder/cardinality.h"

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

void Waerden(Solver &solver, std::vector<int> k, int n, Mode mode) {
  const int b = (int)k.size();
  std::vector<std::vector<Var>> x(n);
  for (int i = 1; i <= n; ++i) {
    for (int j = 0; j < b; ++j) {
      x[i - 1].push_back(
          solver.NewVar("x" + std::to_string(i) + "_" + std::to_string(j)));
    }
  }

  // Each position is assigned a color.
  for (int i = 0; i < n; ++i) {
    std::vector<Lit> pos;
    for (int j = 0; j < b; ++j) {
      pos.push_back(x[i][j]);
    }
    ExactlyOne(solver, pos, mode);
  }

  // No k[j] equally spaced colors are the same.
  for (int d = 1; d < n; ++d) {
    for (int j = 0; j < b; ++j) {
      for (int i = 1; i + (k[j] - 1) * d <= n; ++i) {
        std::vector<Lit> clause;
        for (int a = 0; a < k[j]; ++a) {
          clause.push_back(~x[i + a * d - 1][j]);
        }
        solver.AddClause(clause);
      }
    }
  }
}

} // namespace builder
} // namespace solver
