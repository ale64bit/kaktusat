#include "gtest/gtest.h"

#include "solver/algorithm/algorithm.h"
#include "solver/encoder/cardinality.h"
#include "solver/encoder/encoder.h"
#include "solver/encoder/waerden.h"

TEST(EncoderTest, CardinalityExactlyOne) {
  for (int n = 3; n <= 20; ++n) {
    solver::algorithm::Nop solver;
    std::vector<solver::Lit> x;
    std::vector<solver::Lit> lits;
    for (int i = 1; i <= n; ++i) {
      lits.push_back(solver.NewVar("x" + std::to_string(i)));
    }
    solver::encoder::ExactlyOne(solver, lits,
                                solver::encoder::Mode::kLessVariables);
    EXPECT_EQ(solver.NumVars(), n);
    EXPECT_EQ(solver.NumClauses(), 1 + n * (n - 1) / 2);
  }
  for (int n = 3; n <= 20; ++n) {
    solver::algorithm::Nop solver;
    std::vector<solver::Lit> x;
    std::vector<solver::Lit> lits;
    for (int i = 1; i <= n; ++i) {
      lits.push_back(solver.NewVar("x" + std::to_string(i)));
    }
    solver::encoder::ExactlyOne(solver, lits,
                                solver::encoder::Mode::kLessClauses);
    EXPECT_EQ(solver.NumVars(), n + (n - 3) / 2);
    EXPECT_EQ(solver.NumClauses(), 3 * n - 5);
  }
}

TEST(EncoderTest, CardinalitySAT) {
  using SetupFn =
      std::function<void(solver::Solver &, std::vector<solver::Lit> &, int)>;
  using CheckFn = std::function<void(int, int)>;

  struct EncoderTest {
    SetupFn Setup;
    CheckFn Check;
  };

  for (int n = 2; n <= 10; ++n) {
    for (int r = 0; r <= n; ++r) {
      for (auto tt : {
               EncoderTest{
                   [](solver::Solver &solver, std::vector<solver::Lit> &x,
                      int r) { solver::encoder::AtMost(solver, x, r); },
                   [](int got, int r) { EXPECT_LE(got, r); },
               },
               EncoderTest{
                   [](solver::Solver &solver, std::vector<solver::Lit> &x,
                      int r) { solver::encoder::AtLeast(solver, x, r); },
                   [](int got, int r) { EXPECT_GE(got, r); },
               },
               EncoderTest{
                   [](solver::Solver &solver, std::vector<solver::Lit> &x,
                      int r) { solver::encoder::Exactly(solver, x, r); },
                   [](int got, int r) { EXPECT_EQ(got, r); },
               },
           }) {

        solver::algorithm::Default solver;
        std::vector<solver::Lit> x;
        for (int i = 1; i <= n; ++i) {
          x.push_back(solver.NewVar("x" + std::to_string(i)));
        }

        tt.Setup(solver, x, r);
        auto [res, sol] = solver.Solve();
        ASSERT_EQ(res, solver::Result::kSAT);

        int cnt = 0;
        for (const auto &l : sol) {
          if (!solver.IsTemp(l.V()) && !l.IsNeg()) {
            ++cnt;
          }
        }
        tt.Check(cnt, r);
      }
    }
  }
}

TEST(EncoderTest, Waerden) {
  for (int j = 2; j <= 20; ++j) {
    for (int k = 2; k <= 20; ++k) {
      for (int n = 1; n <= 50; ++n) {
        solver::algorithm::Nop solver;
        solver::encoder::Waerden(solver, j, k, n);
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
