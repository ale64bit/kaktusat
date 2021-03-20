#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

class Z : public Solver {
public:
  Z() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "Z"; }
};

} // namespace algorithm
} // namespace solver
