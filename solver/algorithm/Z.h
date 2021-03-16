#pragma once

#include "solver/solver.h"

#include <vector>

namespace solver {
namespace algorithm {

class Z : public Solver {
public:
  Z() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
};

} // namespace algorithm
} // namespace solver
