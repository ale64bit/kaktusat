#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * @see: https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/satformat.ps
 */
std::string FromDimacsFile(Solver &, std::string);

} // namespace builder
} // namespace solver
