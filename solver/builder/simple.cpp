#include "solver/builder/simple.h"

namespace solver {
namespace builder {

void Unit(Solver &solver) {
  solver.Reset();
  auto x = solver.NewVar("x");
  solver.AddClause({x});
}

void Tautology(Solver &solver) {
  solver.Reset();
  auto x = solver.NewVar("x");
  solver.AddClause({x, ~x});
}

void Contradiction(Solver &solver) {
  solver.Reset();
  auto x = solver.NewVar("x");
  solver.AddClause({x});
  solver.AddClause({~x});
}

// 7.2.2.2 - (6), p4
void R(Solver &solver) {
  Rprime(solver);

  auto x4 = solver.GetVar("x4");
  auto x1 = solver.GetVar("x1");
  auto x2 = solver.GetVar("x2");

  solver.AddClause({~x4, x1, ~x2});
}

// 7.2.2.2 - (7), p4
void Rprime(Solver &solver) {
  solver.Reset();

  auto x1 = solver.NewVar("x1");
  auto x2 = solver.NewVar("x2");
  auto x3 = solver.NewVar("x3");
  auto x4 = solver.NewVar("x4");

  solver.AddClause({x1, x2, ~x3});
  solver.AddClause({x2, x3, ~x4});
  solver.AddClause({x3, x4, x1});
  solver.AddClause({x4, ~x1, x2});
  solver.AddClause({~x1, ~x2, x3});
  solver.AddClause({~x2, ~x3, x4});
  solver.AddClause({~x3, ~x4, ~x1});
}

} // namespace builder
} // namespace solver
