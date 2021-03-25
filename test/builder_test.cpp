#include "gtest/gtest.h"

#include "solver/algorithm/nop.h"
#include "solver/builder/builder.h"
#include "solver/builder/cardinality.h"
#include "solver/builder/waerden.h"

TEST(BuilderTest, Cardinality) {
  for (int n = 3; n <= 100; ++n) {
    solver::algorithm::Nop solver;
    std::vector<solver::Lit> x;
    std::vector<solver::Lit> lits;
    for (int i = 1; i <= n; ++i) {
      lits.push_back(solver.NewVar("x" + std::to_string(i)));
    }
    solver::builder::ExactlyOne(solver, lits,
                                solver::builder::Mode::kLessVariables);
    EXPECT_EQ(solver.NumVars(), n);
    EXPECT_EQ(solver.NumClauses(), 1 + n * (n - 1) / 2);
  }
  for (int n = 3; n <= 100; ++n) {
    solver::algorithm::Nop solver;
    std::vector<solver::Lit> x;
    std::vector<solver::Lit> lits;
    for (int i = 1; i <= n; ++i) {
      lits.push_back(solver.NewVar("x" + std::to_string(i)));
    }
    solver::builder::ExactlyOne(solver, lits,
                                solver::builder::Mode::kLessClauses);
    EXPECT_EQ(solver.NumVars(), n + (n - 3) / 2);
    EXPECT_EQ(solver.NumClauses(), 3 * n - 5);
  }
}

TEST(BuilderTest, Waerden) {
  for (int j = 2; j <= 20; ++j) {
    for (int k = 2; k <= 20; ++k) {
      for (int n = 1; n <= 50; ++n) {
        solver::algorithm::Nop solver;
        solver::builder::Waerden(solver, j, k, n);
        EXPECT_EQ(solver.NumVars(), n);
        EXPECT_EQ(solver.NumClauses(),
                  ((n / (j - 1)) * n -
                   (j - 1) * (n / (j - 1)) * ((n / (j - 1)) + 1) / 2) +
                      ((n / (k - 1)) * n -
                       (k - 1) * (n / (k - 1)) * ((n / (k - 1)) + 1) / 2));
      }
    }
  }
}
