#include "solver/builder/mutilated_chessboard.h"

#include <array>
#include <string>
#include <vector>

#include "solver/builder/cardinality.h"
#include "util/check.h"

namespace solver {
namespace builder {

void MutilatedChessboard(Solver &solver, int n) {
  CHECK("n must be greater than 1", n > 1);
  constexpr std::array<int, 4> di = {-1, 1, 0, 0};
  constexpr std::array<int, 4> dj = {0, 0, -1, 1};

  // Create variables.
  // x(i, j, k) means that we match cell at (i, j) with neighbor in direction k.
  std::vector<std::vector<std::vector<solver::Lit>>> x(
      n, std::vector<std::vector<solver::Lit>>(n));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < 4; ++k) {
        x[i][j].push_back(solver.NewVar("x" + std::to_string(i + 1) + "_" +
                                        std::to_string(j + 1) + "_" +
                                        std::to_string(k)));
      }
    }
  }

  // Add constraints.
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < 4; ++k) {
        const int ii = i + di[k];
        const int jj = j + dj[k];
        if (0 <= ii && ii < n && 0 <= jj && jj < n) {
          // Matched neighbor cells imply each other.
          solver.AddClause({~x[i][j][k], x[ii][jj][k ^ 1]});
          solver.AddClause({~x[ii][jj][k ^ 1], x[i][j][k]});
        } else {
          // Cannot match with cells outside the board.
          solver.AddClause({~x[i][j][k]});
        }
      }
      // Each cell must be matched with a neighbor.
      solver::builder::ExactlyOne(solver, x[i][j]);
    }
  }

  // Constrain the forbidden corners.
  solver.AddClause({~x[0][0][1]});
  solver.AddClause({~x[0][0][3]});
  solver.AddClause({~x[n - 1][n - 1][0]});
  solver.AddClause({~x[n - 1][n - 1][2]});
}

} // namespace builder
} // namespace solver
