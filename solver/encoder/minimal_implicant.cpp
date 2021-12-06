#include "minimal_implicant.h"

#include <algorithm>

#include "solver/encoder/cardinality.h"
#include "solver/encoder/circuit.h"

namespace solver {
namespace encoder {

void MinimalImplicant(Solver &solver, int n, int g) {
  constexpr int K = 3;

  // Variable assignment to the target formula.
  // x[clause][literal][variable][polarity]
  std::vector<Var> x[g + 1][K][n];

  // Constraints on the subsets of the target formula.
  // y[v1][p1][v2][p2][v3][p3][skippedClause][clause][literal][variable][polarity]
  std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<
      std::vector<std::vector<std::vector<std::vector<std::vector<Var>>>>>>>>>>>
      y(n,
        std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<std::vector<Var>>>>>>>>>>(
            2,
            std::vector<std::vector<std::vector<std::vector<std::vector<
                std::vector<std::vector<std::vector<std::vector<Var>>>>>>>>>(
                n,
                std::vector<std::vector<std::vector<std::vector<
                    std::vector<std::vector<std::vector<std::vector<Var>>>>>>>>(
                    2,
                    std::vector<std::vector<std::vector<std::vector<
                        std::vector<std::vector<std::vector<Var>>>>>>>(
                        n,
                        std::vector<std::vector<std::vector<
                            std::vector<std::vector<std::vector<Var>>>>>>(
                            2,
                            std::vector<std::vector<
                                std::vector<std::vector<std::vector<Var>>>>>(
                                g,
                                std::vector<
                                    std::vector<std::vector<std::vector<Var>>>>(
                                    g,
                                    std::vector<std::vector<std::vector<Var>>>(
                                        K, std::vector<std::vector<Var>>(
                                               n))))))))));

  // Create the variables encoding the solution.
  for (int c = 0; c <= g; ++c) {
    for (int l = 0; l < K; ++l) {
      for (int v = 0; v < n; ++v) {
        for (int p = 0; p < 2; ++p) {
          std::string name = "x_" + std::to_string(c) + "_" +
                             std::to_string(l) + "_" + std::to_string(v + 1) +
                             "_" + std::to_string(p);
          x[c][l][v].push_back(solver.NewVar(name));
        }
      }
    }
  }

  // Create the variables encoding the subset contraints.
  for (int v1 = 0; v1 < n; ++v1) {
    for (int p1 = 0; p1 < 2; ++p1) {
      for (int v2 = v1 + 1; v2 < n; ++v2) {
        for (int p2 = 0; p2 < 2; ++p2) {
          for (int v3 = v2 + 1; v3 < n; ++v3) {
            for (int p3 = 0; p3 < 2; ++p3) {
              for (int sc = 0; sc < g; ++sc) {
                for (int c = 0; c < g; ++c) {
                  for (int l = 0; l < K; ++l) {
                    for (int v = 0; v < n; ++v) {
                      for (int p = 0; p < 2; ++p) {
                        std::string name =
                            "y_" + std::to_string(v1) + "_" +
                            std::to_string(p1) + "_" + std::to_string(v2) +
                            "_" + std::to_string(p2) + "_" +
                            std::to_string(v3) + "_" + std::to_string(p3) +
                            "_" + std::to_string(sc) + "_" + std::to_string(c) +
                            "_" + std::to_string(l) + "_" +
                            std::to_string(v + 1) + "_" + std::to_string(p);
                        y[v1][p1][v2][p2][v3][p3][sc][c][l][v].push_back(
                            solver.NewVar(name));
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Each (clause, literal) is assigned exactly one (variable, polarity).
  for (int c = 0; c <= g; ++c) {
    for (int l = 0; l < K; ++l) {
      std::vector<Lit> lits;
      for (int v = 0; v < n; ++v) {
        for (int p = 0; p < 2; ++p) {
          lits.push_back(x[c][l][v][p]);
        }
      }
      ExactlyOne(solver, lits);
    }
  }

  // Each variable is used at most once in each clause.
  for (int c = 0; c <= g; ++c) {
    for (int l1 = 0; l1 < K; ++l1) {
      for (int l2 = l1 + 1; l2 < K; ++l2) {
        for (int v = 0; v < n; ++v) {
          for (int p1 = 0; p1 < 2; ++p1) {
            for (int p2 = 0; p2 < 2; ++p2) {
              solver.AddClause({~x[c][l1][v][p1], ~x[c][l2][v][p2]});
            }
          }
        }
      }
    }
  }

  // Fix subset constraint premise and conclusion clauses to the target clause.
  for (int v1 = 0; v1 < n; ++v1) {
    for (int p1 = 0; p1 < 2; ++p1) {
      for (int v2 = v1 + 1; v2 < n; ++v2) {
        for (int p2 = 0; p2 < 2; ++p2) {
          for (int v3 = v2 + 1; v3 < n; ++v3) {
            for (int p3 = 0; p3 < 2; ++p3) {
              for (int sc = 0; sc < g; ++sc) {
                // premises
                for (int i = 0, c = 0; i < g; ++i) {
                  if (sc == i)
                    continue;
                  for (int l = 0; l < K; ++l) {
                    for (int v = 0; v < n; ++v) {
                      for (int p = 0; p < 2; ++p) {
                        Eq(solver, y[v1][p1][v2][p2][v3][p3][sc][c][l][v][p],
                           x[i][l][v][p]);
                      }
                    }
                  }
                  c++;
                }
                // conclusion
                solver.AddClause(
                    {y[v1][p1][v2][p2][v3][p3][sc][g - 1][0][v1][p1]});
                solver.AddClause(
                    {y[v1][p1][v2][p2][v3][p3][sc][g - 1][1][v2][p2]});
                solver.AddClause(
                    {y[v1][p1][v2][p2][v3][p3][sc][g - 1][2][v3][p3]});
              }
            }
          }
        }
      }
    }
  }

  // Assert that each assignment which satisfies the premises also satisfies the
  // conclusion. Hence the conclusion clause is a tautological consequence of
  // the premise clauses.
  for (int mask = 0; mask < (1 << n); ++mask) {
    std::vector<Lit> implication;
    for (int c = 0; c <= g; ++c) {
      std::vector<Lit> lits;
      for (int l = 0; l < K; ++l) {
        for (int v = 0; v < n; ++v) {
          const int p = (mask & (1 << v)) != 0;
          lits.push_back(x[c][l][v][p]);
        }
      }
      auto t = solver.NewTempVar("c");
      Or(solver, t, lits);
      if (c < g)
        implication.push_back(~t);
      else
        implication.push_back(t);
    }
    solver.AddClause(implication);
  }

  // Assert that no proper subset of the premises tautologically implies the
  // conclusion.
  for (int skip = 0; skip < g; ++skip) {
    std::vector<Lit> notSome;
    for (int mask = 0; mask < (1 << n); ++mask) {
      std::vector<Lit> implication;
      for (int c = 0; c <= g; ++c) {
        if (c == skip)
          continue;
        std::vector<Lit> lits;
        for (int l = 0; l < K; ++l) {
          for (int v = 0; v < n; ++v) {
            const int p = (mask & (1 << v)) != 0;
            lits.push_back(x[c][l][v][p]);
          }
        }
        auto t = solver.NewTempVar("c");
        Or(solver, t, lits);
        if (c < g)
          implication.push_back(~t);
        else
          implication.push_back(t);
      }
      {
        auto t = solver.NewTempVar("t");
        Or(solver, t, implication);
        notSome.push_back(~t);
      }
    }
    solver.AddClause(notSome);
  }

  // Assert that no proper subset of the premises tautologically implies any
  // other clause.
  for (int v1 = 0; v1 < n; ++v1) {
    for (int p1 = 0; p1 < 2; ++p1) {
      for (int v2 = v1 + 1; v2 < n; ++v2) {
        for (int p2 = 0; p2 < 2; ++p2) {
          for (int v3 = v2 + 1; v3 < n; ++v3) {
            for (int p3 = 0; p3 < 2; ++p3) {
              for (int sc = 0; sc < g; ++sc) {
                // For each possible clause and proper subset of premises:
                // y[v1][p1][v2][p2][v3][p3][sc]
                std::vector<Lit> notSome;
                for (int mask = 0; mask < (1 << n); ++mask) {
                  std::vector<Lit> implication;
                  for (int c = 0; c < g; ++c) {
                    std::vector<Lit> lits;
                    for (int l = 0; l < K; ++l) {
                      for (int v = 0; v < n; ++v) {
                        const int p = (mask & (1 << v)) != 0;
                        lits.push_back(
                            y[v1][p1][v2][p2][v3][p3][sc][c][l][v][p]);
                      }
                    }
                    auto t = solver.NewTempVar("c");
                    Or(solver, t, lits);
                    if (c < g - 1)
                      implication.push_back(~t);
                    else
                      implication.push_back(t);
                  }
                  {
                    auto t = solver.NewTempVar("t");
                    Or(solver, t, implication);
                    notSome.push_back(~t);
                  }
                }
                // Exclude cases where one of the premises is the conclusion.
                for (int c = 0; c < g - 1; ++c) {
                  std::array<int, K> lperm{0, 1, 2};
                  do {
                    auto t = solver.NewTempVar("t");
                    And(solver, t,
                        {y[v1][p1][v2][p2][v3][p3][sc][c][lperm[0]][v1][p1],
                         y[v1][p1][v2][p2][v3][p3][sc][c][lperm[1]][v2][p2],
                         y[v1][p1][v2][p2][v3][p3][sc][c][lperm[2]][v3][p3]});
                    notSome.push_back(t);
                  } while (std::next_permutation(lperm.begin(), lperm.end()));
                }
                solver.AddClause(notSome);
              }
            }
          }
        }
      }
    }
  }
}

} // namespace encoder
} // namespace solver
