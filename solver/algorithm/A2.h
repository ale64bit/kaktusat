#pragma once

#include "solver/solver.h"

#include <vector>

namespace solver {
namespace algorithm {

class A2 : public Solver {
public:
  A2() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "A2"; }
};

} // namespace algorithm
} // namespace solver
