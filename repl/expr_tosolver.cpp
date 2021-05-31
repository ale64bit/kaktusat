#include "repl/expr.h"

#include <cassert>

#include "solver/encoder/circuit.h"

solver::Var ConstExpr::ToSolver(solver::Solver &solver) const {
  // Constants should generally not survive evaluation.
  auto ret = solver.NewTempVar();
  if (value_) {
    solver.AddClause({ret});
  } else {
    solver.AddClause({~ret});
  }
  return ret;
}

solver::Var VariableIDExpr::ToSolver(solver::Solver &solver) const {
  return solver.NewOrGetVar(id_);
}

solver::Var FormulaIDExpr::ToSolver(solver::Solver &solver) const {
  // Treat free formulas as variables for sat purposes.
  return solver.NewOrGetVar(id_);
}

solver::Var NegExpr::ToSolver(solver::Solver &solver) const {
  auto ret = solver.NewTempVar();
  auto x = rhs_->ToSolver(solver);
  solver::encoder::Eq(solver, ret, ~x);
  return ret;
}

solver::Var BinExpr::ToSolver(solver::Solver &solver) const {
  auto ret = solver.NewTempVar();
  auto lhs = lhs_->ToSolver(solver);
  auto rhs = rhs_->ToSolver(solver);
  switch (conn_) {
  case BinaryConnective::kAnd:
    solver::encoder::And(solver, ret, lhs, rhs);
    break;
  case BinaryConnective::kOr:
    solver::encoder::Or(solver, ret, lhs, rhs);
    break;
  case BinaryConnective::kXor:
    solver::encoder::Xor(solver, ret, lhs, rhs);
    break;
  case BinaryConnective::kImpl:
    solver::encoder::Or(solver, ret, ~lhs, rhs);
    break;
  case BinaryConnective::kEq: {
    // ret = (~lhs | rhs) & (~rhs | lhs)
    auto t1 = solver.NewTempVar();
    auto t2 = solver.NewTempVar();
    solver::encoder::Or(solver, t1, ~lhs, rhs);
    solver::encoder::Or(solver, t2, ~rhs, lhs);
    solver::encoder::And(solver, ret, t1, t2);
    break;
  }
  }
  return ret;
}

solver::Var SubExpr::ToSolver(solver::Solver &solver) const {
  assert(false); // Substitutions should not survive evaluation.
}
