#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm B (Satisfiability by watching) - p31
 */
class B : public Solver {
public:
  B() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "B"; }
};

} // namespace algorithm
} // namespace solver
