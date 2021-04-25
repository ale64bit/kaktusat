#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "solver/algorithm/a.h"
#include "solver/algorithm/a2.h"
#include "solver/algorithm/b.h"
#include "solver/algorithm/c.h"
#include "solver/algorithm/d.h"
#include "solver/algorithm/i0.h"
#include "solver/algorithm/z.h"
#include "solver/encoder/anti_maximal_element.h"
#include "solver/encoder/cardinality.h"
#include "solver/encoder/coloring.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/factor.h"
#include "solver/encoder/langford.h"
#include "solver/encoder/mutilated_chessboard.h"
#include "solver/encoder/pigeonhole.h"
#include "solver/encoder/sample.h"
#include "solver/encoder/waerden.h"

using BuildFn = std::function<void(solver::Solver &)>;

std::vector<std::unique_ptr<solver::Solver>> AllSolvers() {
  std::vector<std::unique_ptr<solver::Solver>> solvers;
  solvers.emplace_back(new solver::algorithm::A());
  solvers.emplace_back(new solver::algorithm::A2());
  solvers.emplace_back(new solver::algorithm::B());
  solvers.emplace_back(new solver::algorithm::C());
  solvers.emplace_back(new solver::algorithm::D());
  solvers.emplace_back(new solver::algorithm::I0());
  return solvers;
}

std::vector<BuildFn> AllSATEncoders() {
  namespace enc = solver::encoder;
  namespace graph = solver::encoder::graph;
  return {
      [](solver::Solver &s) { enc::Unit(s); },
      [](solver::Solver &s) { enc::Tautology(s); },
      [](solver::Solver &s) { enc::Rprime(s); },
      [](solver::Solver &s) { enc::Waerden(s, 3, 3, 8); },
      [](solver::Solver &s) { enc::Langford(s, 3); },
      [](solver::Solver &s) { enc::Langford(s, 4); },
      [](solver::Solver &s) { enc::Langford(s, 7); },
      [](solver::Solver &s) { enc::Coloring(s, 3, graph::Petersen()); },
      [](solver::Solver &s) { enc::Coloring(s, 4, graph::McGregor3()); },
      [](solver::Solver &s) { enc::Coloring(s, 2, graph::FlowerSnark(4)); },
      [](solver::Solver &s) { enc::Coloring(s, 3, graph::FlowerSnark(5)); },
      [](solver::Solver &s) { enc::Coloring(s, 3, graph::FlowerSnarkLine(4)); },
      [](solver::Solver &s) { enc::Coloring(s, 4, graph::FlowerSnarkLine(5)); },
      [](solver::Solver &s) {
        std::vector<solver::Lit> x;
        for (int i = 1; i <= 7; ++i) {
          x.push_back(s.NewVar("x" + std::to_string(i)));
        }
        enc::AtLeast(s, x, 3);
        enc::AtMost(s, x, 4);
      },
      [](solver::Solver &s) { enc::Factor(s, 2, 3, 21); },
  };
}

std::vector<BuildFn> AllUNSATEncoders() {
  namespace enc = solver::encoder;
  namespace graph = solver::encoder::graph;
  return {
      [](solver::Solver &s) { enc::Contradiction(s); },
      [](solver::Solver &s) { enc::R(s); },
      [](solver::Solver &s) { enc::Waerden(s, 3, 3, 9); },
      [](solver::Solver &s) { enc::Langford(s, 2); },
      [](solver::Solver &s) { enc::Langford(s, 5); },
      [](solver::Solver &s) { enc::Langford(s, 6); },
      [](solver::Solver &s) { enc::Coloring(s, 2, graph::Petersen()); },
      [](solver::Solver &s) { enc::Coloring(s, 3, graph::McGregor3()); },
      [](solver::Solver &s) { enc::Coloring(s, 2, graph::FlowerSnark(5)); },
      [](solver::Solver &s) { enc::Coloring(s, 2, graph::FlowerSnarkLine(4)); },
      [](solver::Solver &s) { enc::Coloring(s, 3, graph::FlowerSnarkLine(5)); },
      [](solver::Solver &s) {
        std::vector<solver::Lit> x;
        for (int i = 1; i <= 7; ++i) {
          x.push_back(s.NewVar("x" + std::to_string(i)));
        }
        solver::encoder::AtLeast(s, x, 4);
        solver::encoder::AtMost(s, x, 3);
      },
      [](solver::Solver &s) { enc::AntiMaximalElement(s, 3); },
      [](solver::Solver &s) { enc::Pigeonhole(s, 3); },
      [](solver::Solver &s) { enc::Pigeonhole(s, 4); },
      [](solver::Solver &s) { enc::MutilatedChessboard(s, 4); },
      [](solver::Solver &s) { enc::MutilatedChessboard(s, 5); },
      [](solver::Solver &s) { enc::Factor(s, 2, 3, 19); },
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
