#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * Naive implementation of Algorithm I.
 *
 * @see: 7.2.2.2 - Algorithm I (Satisfiability by clause learning) - p61
 */
class I0 : public Solver {
public:
  I0() {}

  std::pair<Result, std::vector<Lit>> Solve() override;
  std::string ID() const override { return "I0"; }
};

} // namespace algorithm
} // namespace solver
