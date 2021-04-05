#include "solver/builder/circuit.h"

#include "util/log.h"

namespace solver {
namespace builder {

void Set(Solver &solver, Var t, Var u) {
  solver.AddClause({u, ~t});
  solver.AddClause({~u, t});
}

void And(Solver &solver, Var t, Var u, Var v) {
  solver.AddClause({u, ~t});
  solver.AddClause({v, ~t});
  solver.AddClause({~u, ~v, t});
}

void Or(Solver &solver, Var t, Var u, Var v) {
  solver.AddClause({~u, t});
  solver.AddClause({~v, t});
  solver.AddClause({u, v, ~t});
}

void Xor(Solver &solver, Var t, Var u, Var v) {
  solver.AddClause({~u, v, t});
  solver.AddClause({u, ~v, t});
  solver.AddClause({u, v, ~t});
  solver.AddClause({~u, ~v, ~t});
}

void HalfAdder(Solver &solver, Var t, Var c, Var u, Var v) {
  Xor(solver, t, u, v);
  And(solver, c, u, v);
}

} // namespace builder
} // namespace solver
