#pragma once

#include <array>
#include <string>

#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Sudoku instance encoder.
 *
 * Encodes the given sudoku board. Empty cells are represented with zero.
 * The solution is encoded in variables xi_j_d, meaning that digit d should be
 * written in cell (i, j).
 */
void Sudoku(Solver &, const std::array<std::array<int, 9>, 9> &);

} // namespace encoder
} // namespace solver
