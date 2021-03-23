#include "solver/builder/langford.h"

#include <vector>

#include "solver/builder/cardinality.h"

namespace solver {
namespace builder {

void Langford(Solver &solver, int n) {
  auto VarId = [](int i, int j, int k) {
    return "d" + std::to_string(i) + "s" + std::to_string(j) + "s" +
           std::to_string(k);
  };

  std::vector<std::vector<Lit>> perDigit(n + 1);
  std::vector<std::vector<Lit>> perSlot(2 * n + 1);
  for (int i = 1; i <= n; ++i) {
    for (int j = 1; i + j + 1 <= 2 * n; ++j) {
      int k = i + j + 1;
      // can put digit i in slot j and k
      Var x = solver.NewVar(VarId(i, j, k));
      perDigit[i].push_back(x);
      perSlot[j].push_back(x);
      perSlot[k].push_back(x);
    }
  }

  // Each digit pair must be placed exactly once.
  for (int i = 1; i <= n; ++i) {
    ExactlyOne(solver, perDigit[i]);
  }

  // Each slot must be used exactly once.
  for (int j = 1; j <= 2 * n; ++j) {
    ExactlyOne(solver, perSlot[j]);
  }
}

} // namespace builder
} // namespace solver
