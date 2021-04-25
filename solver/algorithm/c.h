#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm C (Satisfiability by CDCL) - p68
 */
class C : public Solver {
public:
  C() {}

  std::pair<Result, Assignment> Solve() override;
  std::pair<Result, std::vector<Assignment>> SolveAll() override;
  std::string ID() const override { return "C"; }
};

} // namespace algorithm
} // namespace solver
