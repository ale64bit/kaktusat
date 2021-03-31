#include "solver/builder/pigeonhole.h"

#include <string>
#include <vector>

#include "util/check.h"

namespace solver {
namespace builder {

void Pigeonhole(Solver &solver, int m) {
  // x(j, k) means pigeon j takes hole k
  // for 0 <= j <= m, 1 <= k <= m

  // Create variables.
  std::vector<std::vector<solver::Lit>> x(m + 1);
  for (int j = 0; j <= m; ++j) {
    for (int k = 1; k <= m; ++k) {
      x[j].push_back(
          solver.NewVar("x" + std::to_string(j) + "_" + std::to_string(k)));
    }
  }

  // Every pigeon has a hole.
  for (int j = 0; j <= m; ++j) {
    solver.AddClause(x[j]);
  }

  // At most one pigeon in any whole.
  for (int k = 0; k < m; ++k) {
    for (int i = 0; i <= m; ++i) {
      for (int j = i + 1; j <= m; ++j) {
        solver.AddClause({~x[i][k], ~x[j][k]});
      }
    }
  }
}

} // namespace builder
} // namespace solver
