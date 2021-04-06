#include "solver/encoder/sample.h"

#include "solver/encoder/cardinality.h"

namespace solver {
namespace encoder {

void Sudoku(Solver &solver, const std::array<std::array<int, 9>, 9> &t) {
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      CHECK(0 <= t[i][j] && t[i][j] <= 9)
          << "cells can only contain a number from 0 to 9, inclusive";
    }
  }

  // Create variables.
  // x(i, j, d) means we put digit d in cell at (i, j).
  std::vector<std::vector<std::vector<Lit>>> x(
      9, std::vector<std::vector<Lit>>(9));
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      for (int d = 1; d <= 9; ++d) {
        x[i][j].push_back(solver.NewVar("x" + std::to_string(i + 1) + "_" +
                                        std::to_string(j + 1) + "_" +
                                        std::to_string(d)));
      }
    }
  }

  // Each cell must be filled.
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      AtLeastOne(solver, x[i][j]);
    }
  }

  // Add row and column constraints.
  for (int d = 1; d <= 9; ++d) {
    // Each digit must appear exactly once in each row.
    for (int i = 0; i < 9; ++i) {
      std::vector<Lit> row;
      for (int j = 0; j < 9; ++j) {
        row.push_back(x[i][j][d - 1]);
      }
      ExactlyOne(solver, row);
    }
    // Each digit must appear exactly once in each column.
    for (int j = 0; j < 9; ++j) {
      std::vector<Lit> col;
      for (int i = 0; i < 9; ++i) {
        col.push_back(x[i][j][d - 1]);
      }
      ExactlyOne(solver, col);
    }
  }

  // Add 3x3 square constraints.
  for (int d = 1; d <= 9; ++d) {
    for (int di = 0; di < 9; di += 3) {
      for (int dj = 0; dj < 9; dj += 3) {
        std::vector<Lit> sqr;
        for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < 3; ++j) {
            sqr.push_back(x[i + di][j + dj][d - 1]);
          }
        }
        ExactlyOne(solver, sqr);
      }
    }
  }

  // Add input unit constraints.
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      if (t[i][j] > 0) {
        solver.AddClause({x[i][j][t[i][j] - 1]});
      }
    }
  }
}

} // namespace encoder
} // namespace solver
