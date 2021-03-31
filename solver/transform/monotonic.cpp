#include "solver/transform/monotonic.h"

#include <string>
#include <vector>

#include "util/check.h"

namespace solver {
namespace transform {

void Monotonic(Solver &solver) {
  const int n = solver.NumVars();
  const int m = solver.NumClauses();
  std::vector<std::string> names = solver.GetVarNames();
  std::vector<std::vector<Lit>> clauses = solver.GetClauses();

  solver.Reset();
  std::vector<Var> posVars;
  std::vector<Var> negVars;
  for (auto name : names) {
    auto x0 = solver.NewVar(name + "_0");
    auto x1 = solver.NewVar(name + "_1");
    posVars.push_back(x0);
    negVars.push_back(x1);
    solver.AddClause({~x0, ~x1});
  }
  for (auto clause : clauses) {
    std::vector<Lit> newClause;
    for (auto l : clause) {
      if (l.IsNeg()) {
        newClause.push_back(negVars[l.V().ID() - 1]);
      } else {
        newClause.push_back(posVars[l.V().ID() - 1]);
      }
    }
    solver.AddClause(newClause);
  }

  CHECK("Number of variables should be 2n", solver.NumVars() == 2 * n);
  CHECK("Number of clauses should be m+n", solver.NumClauses() == m + n);
}

} // namespace transform
} // namespace solver
