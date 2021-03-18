#pragma once

#include "solver/solver.h"

#include <vector>

namespace solver {
namespace algorithm {

class B : public Solver {
public:
  B() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "B"; }
};

} // namespace algorithm
} // namespace solver
