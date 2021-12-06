#include "solver/encoder/circuit.h"

#include "util/log.h"

namespace solver {
namespace encoder {

// u -> t
// ~u -> ~t
void Eq(Solver &solver, Lit t, Lit u) {
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

void And(Solver &solver, Lit t, const std::vector<Lit> &us) {
  if (us.size() == 1) {
    Eq(solver, t, us[0]);
  } else if (us.size() == 2) {
    And(solver, t, us[0], us[1]);
  } else {
    Lit w = solver.NewTempVar("w");
    Eq(solver, w, us[0]);
    for (size_t i = 1; i < us.size() - 1; ++i) {
      Lit ww = solver.NewTempVar("w");
      And(solver, ww, w, us[i]);
      w = ww;
    }
    And(solver, t, w, us.back());
  }
}

// u -> t
// v -> t
// ~u & ~v -> ~t
void Or(Solver &solver, Lit t, Lit u, Lit v) {
  solver.AddClause({~u, t});
  solver.AddClause({~v, t});
  solver.AddClause({u, v, ~t});
}

void Or(Solver &solver, Lit t, const std::vector<Lit> &us) {
  if (us.size() == 1) {
    Eq(solver, t, us[0]);
  } else if (us.size() == 2) {
    Or(solver, t, us[0], us[1]);
  } else {
    Lit w = solver.NewTempVar("w");
    Eq(solver, w, us[0]);
    for (size_t i = 1; i < us.size() - 1; ++i) {
      Lit ww = solver.NewTempVar("w");
      Or(solver, ww, w, us[i]);
      w = ww;
    }
    Or(solver, t, w, us.back());
  }
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

// @see: 7.1.2 - (23), p107
static void FullAdder1(Solver &solver, Lit t, Lit c1, Lit u, Lit v, Lit c0) {
  auto w = solver.NewTempVar("w");
  // t = u ^ v ^ c0
  Xor(solver, w, u, v);
  Xor(solver, t, w, c0);
  // c1 = median(u, v, c0) = (u & v) | (c0 & (u ^ v))
  auto y1 = solver.NewTempVar("y1");
  auto y2 = solver.NewTempVar("y2");
  And(solver, y1, c0, w);
  And(solver, y2, u, v);
  Or(solver, c1, y1, y2);
}

[[maybe_unused]] static void FullAdder2(Solver &solver, Lit t, Lit c1, Lit u,
                                        Lit v, Lit c0) {
  // Use the direct Tseytin encoding.
  // t = u ^ v ^ c0
  solver.AddClause({u, v, c0, ~t});
  solver.AddClause({u, v, ~c0, t});
  solver.AddClause({u, ~v, c0, t});
  solver.AddClause({u, ~v, ~c0, ~t});
  solver.AddClause({~u, v, c0, t});
  solver.AddClause({~u, v, ~c0, ~t});
  solver.AddClause({~u, ~v, c0, ~t});
  solver.AddClause({~u, ~v, ~c0, t});
  // c1 = median(u, v, c0)
  solver.AddClause({u, v, ~c1});
  solver.AddClause({u, c0, ~c1});
  solver.AddClause({v, c0, ~c1});
  solver.AddClause({~u, ~v, c1});
  solver.AddClause({~u, ~c0, c1});
  solver.AddClause({~v, ~c0, c1});
}

void FullAdder(Solver &solver, Lit t, Lit c1, Lit u, Lit v, Lit c0) {
  FullAdder1(solver, t, c1, u, v, c0);
}

} // namespace encoder
} // namespace solver
