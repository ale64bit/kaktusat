#include "solver/encoder/anti_maximal_element.h"

#include <string>
#include <vector>

#include "util/log.h"

namespace solver {
namespace encoder {

void AntiMaximalElement(Solver &solver, int m) {
  CHECK(m > 0) << "there must be at least one element";

  std::vector<std::vector<Var>> x(m);
  for (int i = 1; i <= m; ++i) {
    for (int j = 1; j <= m; ++j) {
      x[i - 1].push_back(
          solver.NewVar("x" + std::to_string(i) + "_" + std::to_string(j)));
    }
  }

  for (int i = 0; i < m; ++i) {
    // non-reflexive
    solver.AddClause({~x[i][i]});
  }

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < m; ++j) {
      for (int k = 0; k < m; ++k) {
        if (i != j && j != k) {
          // transitive: x(i,j) /\ x(j,k) -> x(i,k)
          solver.AddClause({~x[i][j], ~x[j][k], x[i][k]});
        }
      }
    }
  }

  // Each element has a successor.
  for (int i = 0; i < m; ++i) {
    std::vector<Lit> clause;
    for (int j = 0; j < m; ++j) {
      if (i != j) {
        clause.push_back(x[i][j]);
      }
    }
    solver.AddClause(clause);
  }
}

} // namespace encoder
} // namespace solver
