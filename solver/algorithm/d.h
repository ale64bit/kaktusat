#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm D (Satisfiability by cyclic DPLL) - p33
 */
class D : public Solver {
public:
  D() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "D"; }
};

} // namespace algorithm
} // namespace solver
