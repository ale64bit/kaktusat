#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/* The slowest solver in the west. */
class Z : public Solver {
public:
  Z() {}

  std::pair<Result, Assignment> Solve() override;
  std::pair<Result, std::vector<Assignment>> SolveAll() override;
  std::string ID() const override { return "Z"; }
};

} // namespace algorithm
} // namespace solver
