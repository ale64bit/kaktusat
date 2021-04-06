#include "solver/encoder/encoder.h"

#include "util/log.h"

namespace solver {
namespace encoder {

void Unit(Solver &solver, std::string id) {
  auto x = solver.NewVar(id);
  solver.AddClause({x});
}

void Tautology(Solver &solver, std::string id) {
  auto x = solver.NewVar(id);
  solver.AddClause({x, ~x});
}

void Contradiction(Solver &solver, std::string id) {
  auto x = solver.NewVar(id);
  solver.AddClause({x});
  solver.AddClause({~x});
}

} // namespace encoder
} // namespace solver
