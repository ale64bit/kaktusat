#pragma once

#include "solver/solver.h"

#include <vector>

namespace solver {
namespace algorithm {

class A : public Solver {
public:
  A() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
};

} // namespace algorithm
} // namespace solver
