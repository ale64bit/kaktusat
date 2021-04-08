#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * A "solver" that analyzes instances, rather than solving them.
 */
class Analyze : public Solver {
public:
  Analyze() {}

  std::pair<Result, Assignment> Solve() override;
  std::pair<Result, std::vector<Assignment>> SolveAll() override;
  std::string ID() const override { return "Analyze"; }
};

} // namespace algorithm
} // namespace solver
