#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

class D : public Solver {
public:
  D() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "D"; }
};

} // namespace algorithm
} // namespace solver
