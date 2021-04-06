#include "solver/encoder/sample.h"

namespace solver {
namespace encoder {

void R(Solver &solver) {
  Rprime(solver);

  auto x4 = solver.GetVar("x4");
  auto x1 = solver.GetVar("x1");
  auto x2 = solver.GetVar("x2");

  solver.AddClause({~x4, x1, ~x2});
}

void Rprime(Solver &solver) {
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

} // namespace encoder
} // namespace solver
