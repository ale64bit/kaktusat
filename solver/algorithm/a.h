#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

class A : public Solver {
public:
  A() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "A"; }
};

} // namespace algorithm
} // namespace solver
