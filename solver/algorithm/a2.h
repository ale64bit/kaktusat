#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm A (Satisfiability by backtracking) - p28
 *
 * This was my original implementation, which doesn't remove the literals from
 * deactivated clauses but instead uses the order of literals inside clauses to
 * maintain whether they are active or not.
 */
class A2 : public Solver {
public:
  A2() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "A2"; }
};

} // namespace algorithm
} // namespace solver
