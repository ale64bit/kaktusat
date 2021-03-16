#include "gtest/gtest.h"

#include <memory>

#include "solver/algorithm/A.h"
#include "solver/algorithm/Z.h"
#include "solver/builder/simple.h"

std::vector<std::unique_ptr<solver::Solver>> AllSolvers() {
  std::vector<std::unique_ptr<solver::Solver>> solvers;
  solvers.emplace_back(new solver::algorithm::A());
  solvers.emplace_back(new solver::algorithm::Z());
  return solvers;
}

TEST(SimpleTest, SATTest) {
  for (auto &solver : AllSolvers()) {
    solver::builder::Rprime(*solver);
    auto [res, sol] = solver->Solve();
    EXPECT_EQ(res, solver::Result::kSAT);
    EXPECT_TRUE(solver->Verify(sol));
  }
}

TEST(SimpleTest, UNSATTest) {
  for (auto &solver : AllSolvers()) {
    solver::builder::R(*solver);
    auto [res, sol] = solver->Solve();
    EXPECT_EQ(res, solver::Result::kUNSAT);
    EXPECT_TRUE(sol.empty());
  }
}
