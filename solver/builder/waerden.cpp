#include "solver/builder/waerden.h"

#include <vector>

namespace solver {
namespace builder {

/*
 * Instance builder for van der Waerden numbers.
 *
 * waerden(j, k, n) generates an instance that is satisfiable iff n < W(j, k).
 * In other words, it computes a binary sequence of length n such that there are
 * no j equally spaced 0s and no k equally spaced 1s.
 *
 * @see: 7.2.2.2 - (10), p4
 * @see: https://en.wikipedia.org/wiki/Van_der_Waerden_number
 */
void Waerden(Solver &solver, int j, int k, int n) {
  solver.Reset();

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
