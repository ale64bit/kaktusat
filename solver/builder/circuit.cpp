#include "solver/builder/circuit.h"

#include "util/log.h"

namespace solver {
namespace builder {

// u -> t
// ~u -> ~t
void Set(Solver &solver, Lit t, Lit u) {
  solver.AddClause({u, ~t});
  solver.AddClause({~u, t});
}

// ~u -> ~t
// ~v -> ~t
// u & v -> t
void And(Solver &solver, Lit t, Lit u, Lit v) {
  solver.AddClause({u, ~t});
  solver.AddClause({v, ~t});
  solver.AddClause({~u, ~v, t});
}

// u -> t
// v -> t
// ~u & ~v -> ~t
void Or(Solver &solver, Lit t, Lit u, Lit v) {
  solver.AddClause({~u, t});
  solver.AddClause({~v, t});
  solver.AddClause({u, v, ~t});
}

void Xor(Solver &solver, Lit t, Lit u, Lit v) {
  solver.AddClause({~u, v, t});
  solver.AddClause({u, ~v, t});
  solver.AddClause({u, v, ~t});
  solver.AddClause({~u, ~v, ~t});
}

void HalfAdder(Solver &solver, Lit t, Lit c, Lit u, Lit v) {
  Xor(solver, t, u, v);
  And(solver, c, u, v);
}

void FullAdder(Solver &solver, Lit t, Lit c1, Lit u, Lit v, Lit c0) {
  auto w = solver.NewTempVar("tmp");
  // Compute sum bit.
  Xor(solver, w, u, v);
  Xor(solver, t, w, c0);
  // Compute output carry bit.
  solver.AddClause({u, v, ~c1});
  solver.AddClause({u, c0, ~c1});
  solver.AddClause({v, c0, ~c1});
  solver.AddClause({~u, ~v, c1});
  solver.AddClause({~u, ~c0, c1});
  solver.AddClause({~v, ~c0, c1});
}

} // namespace builder
} // namespace solver
