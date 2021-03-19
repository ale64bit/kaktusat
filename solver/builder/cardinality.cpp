#include "solver/builder/cardinality.h"

namespace solver {
namespace builder {

void ExactlyOne(Solver &solver, const std::vector<Lit> &lits) {
  // At least one.
  solver.AddClause(lits);
  // But not two.
  for (size_t i = 0; i < lits.size(); ++i) {
    for (size_t j = i + 1; j < lits.size(); ++j) {
      solver.AddClause({~lits[i], ~lits[j]});
    }
  }
}

} // namespace builder
} // namespace solver
