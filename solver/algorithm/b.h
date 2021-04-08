#pragma once

#include <string>
#include <vector>

#include "solver/solver.h"

namespace solver {
namespace algorithm {

/*
 * 7.2.2.2 - Algorithm B (Satisfiability by watching) - p31
 */
class B : public Solver {
public:
  B() {}

  std::pair<Result, Assignment> Solve() override;
  std::pair<Result, std::vector<Assignment>> SolveAll() override;
  std::string ID() const override { return "B"; }

private:
  Result SolveInternal(std::vector<Assignment> &, bool);
};

} // namespace algorithm
} // namespace solver
