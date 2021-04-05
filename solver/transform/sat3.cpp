#include "solver/transform/sat3.h"

#include <string>
#include <vector>

#include "solver/builder/cardinality.h"

namespace solver {
namespace transform {

void SAT3(Solver &solver) {
  std::vector<std::string> names = solver.GetVarNames();
  std::vector<std::vector<Lit>> clauses = solver.GetClauses();

  solver.Reset();
  for (auto name : names) {
    solver.NewVar(name);
  }
  for (auto clause : clauses) {
    solver::builder::AtLeast(solver, clause, 1);
  }
}

} // namespace transform
} // namespace solver
