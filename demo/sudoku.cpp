#include <cstdlib>
#include <iostream>
#include <string>

#include "solver/algorithm/d.h"
#include "solver/encoder/sudoku.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
  util::InitLogging();

  // Read the input board. Empty cells are represented with zero.
  std::cout << "type the input sudoku:" << std::endl;
  std::array<std::array<int, 9>, 9> t;
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      std::cin >> t[i][j];
    }
  }

  solver::algorithm::D solver;

  // Use the sudoku encoder.
  solver::encoder::Sudoku(solver, t);
  std::cout << "resulting instance has " << solver.NumVars()
            << " variables and " << solver.NumClauses() << " clauses"
            << std::endl;

  // Solve the instance.
  auto [res, sol] = solver.Solve();
  switch (res) {
  case solver::Result::kSAT:
    for (const auto &l : sol) {
      int i, j, d;
      if (!solver.IsTemp(l.V()) && l.IsPos() &&
          sscanf(solver.NameOf(l.V()).c_str(), "x%d_%d_%d", &i, &j, &d) == 3) {
        t[i - 1][j - 1] = d;
      }
    }
    std::cout << "solution found:" << '\n';
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        std::cout << t[i][j] << ' ';
      }
      std::cout << '\n';
    }
    std::cout << std::endl;
    break;
  case solver::Result::kUNSAT:
    std::cout << "sudoku has no solution" << std::endl;
    break;
  case solver::Result::kUnknown:
    std::cout << "instance could not be solved" << std::endl;
    break;
  }

  return 0;
}
