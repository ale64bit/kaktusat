#pragma once

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/*
 * Instance encoder for mutilated chessboard problem of size n.
 *
 * Always unsatisfiable. Useful for refutation experiments.
 *
 * @see: https://en.wikipedia.org/wiki/Mutilated_chessboard_problem
 */
void MutilatedChessboard(Solver &, int n);

} // namespace encoder
} // namespace solver
