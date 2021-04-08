#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "solver/algorithm/a.h"
#include "solver/algorithm/a2.h"
#include "solver/algorithm/b.h"
#include "solver/algorithm/d.h"
#include "solver/algorithm/i0.h"
#include "solver/algorithm/z.h"
#include "solver/encoder/cardinality.h"
#include "solver/encoder/coloring.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/factor.h"
#include "solver/encoder/langford.h"
#include "solver/encoder/mutilated_chessboard.h"
#include "solver/encoder/partial_order.h"
#include "solver/encoder/pigeonhole.h"
#include "solver/encoder/sample.h"
#include "solver/encoder/waerden.h"

using BuildFn = std::function<void(solver::Solver &)>;

std::vector<std::unique_ptr<solver::Solver>> AllSolvers() {
  std::vector<std::unique_ptr<solver::Solver>> solvers;
  solvers.emplace_back(new solver::algorithm::A());
  solvers.emplace_back(new solver::algorithm::A2());
  solvers.emplace_back(new solver::algorithm::B());
  solvers.emplace_back(new solver::algorithm::D());
  solvers.emplace_back(new solver::algorithm::I0());
  // solvers.emplace_back(new solver::algorithm::Z());
  return solvers;
}

std::vector<BuildFn> AllSATEncoders() {
  return {
      [](solver::Solver &s) { solver::encoder::Unit(s); },
      [](solver::Solver &s) { solver::encoder::Tautology(s); },
      [](solver::Solver &s) { solver::encoder::Rprime(s); },
      [](solver::Solver &s) { solver::encoder::Waerden(s, 3, 3, 8); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 3); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 4); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 7); },
      [](solver::Solver &s) {
        solver::encoder::Coloring(s, 3, solver::encoder::graph::Petersen());
      },
      [](solver::Solver &s) {
        solver::encoder::Coloring(s, 4, solver::encoder::graph::McGregor3());
      },
      [](solver::Solver &s) {
        std::vector<solver::Lit> x;
        for (int i = 1; i <= 7; ++i) {
          x.push_back(s.NewVar("x" + std::to_string(i)));
        }
        solver::encoder::AtLeast(s, x, 3);
        solver::encoder::AtMost(s, x, 4);
      },
      [](solver::Solver &s) { solver::encoder::Factor(s, 2, 3, 21); },
  };
}

std::vector<BuildFn> AllUNSATEncoders() {
  return {
      [](solver::Solver &s) { solver::encoder::Contradiction(s); },
      [](solver::Solver &s) { solver::encoder::R(s); },
      [](solver::Solver &s) { solver::encoder::Waerden(s, 3, 3, 9); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 2); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 5); },
      [](solver::Solver &s) { solver::encoder::Langford(s, 6); },
      [](solver::Solver &s) {
        solver::encoder::Coloring(s, 2, solver::encoder::graph::Petersen());
      },
      [](solver::Solver &s) {
        solver::encoder::Coloring(s, 3, solver::encoder::graph::McGregor3());
      },
      [](solver::Solver &s) {
        std::vector<solver::Lit> x;
        for (int i = 1; i <= 7; ++i) {
          x.push_back(s.NewVar("x" + std::to_string(i)));
        }
        solver::encoder::AtLeast(s, x, 4);
        solver::encoder::AtMost(s, x, 3);
      },
      [](solver::Solver &s) { solver::encoder::ImpossiblePartialOrder(s, 3); },
      [](solver::Solver &s) { solver::encoder::Pigeonhole(s, 3); },
      [](solver::Solver &s) { solver::encoder::Pigeonhole(s, 4); },
      [](solver::Solver &s) { solver::encoder::MutilatedChessboard(s, 4); },
      [](solver::Solver &s) { solver::encoder::MutilatedChessboard(s, 5); },
      [](solver::Solver &s) { solver::encoder::Factor(s, 2, 3, 19); },
  };
}

TEST(SolverTest, SATTest) {
  for (auto encoder : AllSATEncoders()) {
    for (auto &solver : AllSolvers()) {
      solver->Reset();
      encoder(*solver);
      auto [res, sol] = solver->Solve();
      EXPECT_EQ(res, solver::Result::kSAT);
      EXPECT_TRUE(solver->Verify(sol));
    }
  }
}

TEST(SolverTest, UNSATTest) {
  for (auto encoder : AllUNSATEncoders()) {
    for (auto &solver : AllSolvers()) {
      solver->Reset();
      encoder(*solver);
      auto [res, sol] = solver->Solve();
      EXPECT_EQ(res, solver::Result::kUNSAT);
      EXPECT_TRUE(sol.empty());
    }
  }
}
