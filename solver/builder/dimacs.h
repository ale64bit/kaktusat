#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for an instance stored in a DIMACS format file.
 *
 * @see: https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/satformat.ps
 */
std::string FromDimacsFile(Solver &, std::string);

} // namespace builder
} // namespace solver
