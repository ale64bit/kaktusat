#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * A dummy solver that doesn't actually solve anything.
 *
 * Useful for testing builder/solver intrinsic properties.
 */
class Nop : public Solver {
public:
  Nop() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "NOP"; }
};

} // namespace algorithm
} // namespace solver
