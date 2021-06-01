#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "solver/algorithm/algorithm.h"
#include "solver/encoder/cardinality.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
  util::InitLogging();

  const std::vector<std::string> kGoal = {
      "...............", //
      ".#...#.###.###.", //
      ".#...#.#...#...", //
      ".#...#.##..##..", //
      ".#...#.#...#...", //
      ".###.#.#...###.", //
      "...............", //
  };

  solver::algorithm::Default solver;

  const int T = 4;
  const int R = static_cast<int>(kGoal.size());
  const int C = static_cast<int>(kGoal[0].size());

  // Encode the problem.
  // x[t][i][j] is the value of the (i,j) cell at time t.
  std::vector<std::vector<std::vector<solver::Var>>> x;
  for (int t = 0; t <= T; ++t) {
    x.push_back({});
    for (int i = 0; i < R; ++i) {
      x[t].push_back({});
      for (int j = 0; j < C; ++j) {
        x[t][i].push_back(solver.NewVar("x" + std::to_string(t) + "_" +
                                        std::to_string(i) + "_" +
                                        std::to_string(j)));
      }
    }
  }

  // Add boundary conditions so that patterns do not escape the grid.
  for (int t = 0; t < T; ++t) {
    for (int i = 2; i < R; ++i) {
      for (auto j : {0, C - 1}) {
        solver.AddClause({~x[t][i - 2][j], ~x[t][i - 1][j], ~x[t][i][j]});
      }
    }
    for (auto i : {0, R - 1}) {
      for (int j = 2; j < C; ++j) {
        solver.AddClause({~x[t][i][j - 2], ~x[t][i][j - 1], ~x[t][i][j]});
      }
    }
  }

  // Add transition constraints.
  for (int t = 1; t <= T; ++t) {
    for (int i = 0; i < R; ++i) {
      for (int j = 0; j < C; ++j) {
        // Collect cell neighbors.
        std::vector<solver::Var> y;
        for (int di = -1; di <= 1; ++di) {
          for (int dj = -1; dj <= 1; ++dj) {
            if (di == 0 && dj == 0) {
              continue;
            }
            if (0 <= i + di && i + di < R && 0 <= j + dj && j + dj < C) {
              y.push_back(x[t - 1][i + di][j + dj]);
            }
          }
        }

        // Add model transitions.

        // 4 or more live neighbors imply the cell dies.
        for (size_t a = 0; a < y.size(); ++a) {
          for (size_t b = a + 1; b < y.size(); ++b) {
            for (size_t c = b + 1; c < y.size(); ++c) {
              for (size_t d = c + 1; d < y.size(); ++d) {
                solver.AddClause({~y[a], ~y[b], ~y[c], ~y[d], ~x[t][i][j]});
              }
            }
          }
        }

        // 3 live neighbors imply the cell lives.
        for (size_t a = 0; a < y.size(); ++a) {
          for (size_t b = a + 1; b < y.size(); ++b) {
            for (size_t c = b + 1; c < y.size(); ++c) {
              solver::Clause clause = {~y[a], ~y[b], ~y[c], x[t][i][j]};
              for (size_t d = 0; d < y.size(); ++d) {
                if (a != d && b != d && c != d) {
                  clause.push_back(y[d]);
                }
              }
              solver.AddClause(clause);
            }
          }
        }

        // 2 live neighbors: the cell lives iff it's currently alive.
        for (size_t a = 0; a < y.size(); ++a) {
          for (size_t b = a + 1; b < y.size(); ++b) {
            solver::Clause lives = {~y[a], ~y[b], ~x[t - 1][i][j], x[t][i][j]};
            solver::Clause dies = {~y[a], ~y[b], x[t - 1][i][j], ~x[t][i][j]};
            for (size_t c = 0; c < y.size(); ++c) {
              if (a != c && b != c) {
                lives.push_back(y[c]);
                dies.push_back(y[c]);
              }
            }
            solver.AddClause(lives);
            solver.AddClause(dies);
          }
        }

        // 1 live neighbor implies the cell dies.
        for (size_t a = 0; a < y.size(); ++a) {
          solver::Clause clause = {~y[a], ~x[t][i][j]};
          for (size_t b = 0; b < y.size(); ++b) {
            if (a != b) {
              clause.push_back(y[b]);
            }
          }
          solver.AddClause(clause);
        }

        // no live neighbors imply the cell dies.
        {
          solver::Clause clause = {~x[t][i][j]};
          for (size_t a = 0; a < y.size(); ++a) {
            clause.push_back(y[a]);
          }
          solver.AddClause(clause);
        }
      }
    }
  }

  // Add constraints on the initial state.
  {
    solver::Clause lits;
    for (int i = 0; i < R; ++i) {
      for (int j = 0; j < C; ++j) {
        lits.push_back(x[0][i][j]);
      }
    }
    solver::encoder::AtMost(solver, lits, 39);
  }
  // Add constraints on the final state.
  for (int i = 0; i < R; ++i) {
    for (int j = 0; j < C; ++j) {
      solver.AddClause({(kGoal[i][j] == '#') ? x[T][i][j] : ~x[T][i][j]});
    }
  }

  std::cout << "resulting instance has " << solver.NumVars()
            << " variables and " << solver.NumClauses() << " clauses"
            << std::endl;

  // Solve the instance.
  int liveCells = 0;
  std::vector<std::string> x0(R, std::string(C, '.'));
  auto [res, sol] = solver.Solve();
  switch (res) {
  case solver::Result::kSAT:
    std::cout << "solution found!" << std::endl;
    for (const auto &l : sol) {
      if (l.IsPos() && !solver.IsTemp(l.V())) {
        std::string name = solver.NameOf(l.V());
        int t, i, j;
        if (sscanf(name.c_str(), "x%d_%d_%d", &t, &i, &j) == 3) {
          if (t == 0) {
            x0[i][j] = '#';
            ++liveCells;
          }
        }
      }
    }
    std::cout << "initial live cells: " << liveCells << '\n';
    for (int i = 0; i < R; ++i) {
      std::cout << x0[i] << '\n';
    }
    break;
  case solver::Result::kUNSAT:
    std::cout << "no solution found" << std::endl;
    break;
  case solver::Result::kUnknown:
    std::cout << "instance could not be solved" << std::endl;
    break;
  }

  return 0;
}
