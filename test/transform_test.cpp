#include "gtest/gtest.h"

#include <array>

#include "solver/algorithm/nop.h"
#include "solver/builder/rand.h"
#include "solver/transform/monotonic.h"
#include "solver/transform/sat3.h"

TEST(TransformTest, Monotonic) {
  solver::algorithm::Nop solver;
  solver::builder::Rand(solver, 8, 10, 3);
  solver::transform::Monotonic(solver);

  for (const auto &clause : solver.GetClauses()) {
    std::array<bool, 2> has{false, false};
    for (const auto &l : clause) {
      has[l.IsNeg()] = true;
    }
    EXPECT_GT(clause.size(), 0);
    EXPECT_FALSE(has[false] && has[true]);
  }
}

TEST(TransformTest, SAT3) {
  solver::algorithm::Nop solver;
  solver::builder::Rand(solver, 8, 10, 7);
  solver::transform::SAT3(solver);

  for (const auto &clause : solver.GetClauses()) {
    EXPECT_GT(clause.size(), 0);
    EXPECT_LE(clause.size(), 3);
  }
}
