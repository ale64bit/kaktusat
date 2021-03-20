#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

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
