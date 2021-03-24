#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm A (Satisfiability by backtracking) - p28
 * 7.2.2.2 - Exercise 121 - p143, p208
 */
class A : public Solver {
public:
  A() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "A"; }
};

} // namespace algorithm
} // namespace solver
