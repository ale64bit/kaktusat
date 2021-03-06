#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * A dummy solver that doesn't actually solve anything.
 *
 * Useful for testing encoder/solver intrinsic properties.
 */
class Nop : public Solver {
public:
  Nop() {}

  std::pair<Result, Assignment> Solve() override;
  std::pair<Result, std::vector<Assignment>> SolveAll() override;
  std::string ID() const override { return "NOP"; }
};

} // namespace algorithm
} // namespace solver
