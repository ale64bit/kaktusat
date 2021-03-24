#include "solver/builder/simple.h"

#include "util/check.h"

namespace solver {
namespace builder {

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

void Monotonize(Solver &solver) {
  const int n = solver.NumVars();
  const int m = solver.NumClauses();
  std::vector<std::string> vars = solver.GetVars();
  std::vector<std::vector<Lit>> clauses = solver.GetClauses();

  solver.Reset();
  std::vector<Var> posVars;
  std::vector<Var> negVars;
  for (auto name : vars) {
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

} // namespace builder
} // namespace solver
