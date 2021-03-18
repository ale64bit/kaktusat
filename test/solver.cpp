#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "solver/algorithm/A.h"
#include "solver/algorithm/A2.h"
#include "solver/algorithm/B.h"
#include "solver/algorithm/Z.h"
#include "solver/builder/simple.h"
#include "solver/builder/waerden.h"

using BuildFn = std::function<void(solver::Solver &)>;

std::vector<std::unique_ptr<solver::Solver>> AllSolvers() {
  std::vector<std::unique_ptr<solver::Solver>> solvers;
  solvers.emplace_back(new solver::algorithm::A());
  solvers.emplace_back(new solver::algorithm::A2());
  solvers.emplace_back(new solver::algorithm::B());
  solvers.emplace_back(new solver::algorithm::Z());
  return solvers;
}

std::vector<BuildFn> AllSATBuilders() {
  return {
      [](solver::Solver &s) { solver::builder::Unit(s); },
      [](solver::Solver &s) { solver::builder::Tautology(s); },
      [](solver::Solver &s) { solver::builder::Rprime(s); },
      [](solver::Solver &s) { solver::builder::waerden(s, 3, 3, 8); },
  };
}

std::vector<BuildFn> AllUNSATBuilders() {
  return {
      [](solver::Solver &s) { solver::builder::Contradiction(s); },
      [](solver::Solver &s) { solver::builder::R(s); },
      [](solver::Solver &s) { solver::builder::waerden(s, 3, 3, 9); },
  };
}

TEST(SolverTest, SATTest) {
  for (auto builder : AllSATBuilders()) {
    for (auto &solver : AllSolvers()) {
      builder(*solver);
      auto [res, sol] = solver->Solve();
      EXPECT_EQ(res, solver::Result::kSAT);
      EXPECT_TRUE(solver->Verify(sol));
    }
  }
}

TEST(SolverTest, UNSATTest) {
  for (auto builder : AllUNSATBuilders()) {
    for (auto &solver : AllSolvers()) {
      builder(*solver);
      auto [res, sol] = solver->Solve();
      EXPECT_EQ(res, solver::Result::kUNSAT);
      EXPECT_TRUE(sol.empty());
    }
  }
}
