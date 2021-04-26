#pragma once

#include <filesystem>
#include <string>

#include "solver/solver.h"

namespace solver {
namespace encoder {

namespace fs = std::filesystem;

/*
 * Instance encoder for an instance stored in a DIMACS format file.
 *
 * @see: https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/satformat.ps
 */
std::string FromDimacsFile(Solver &, fs::path);

/*
 * Encoder to write an existing solver's instance to a DIMACS format file.
 *
 * @see: https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/satformat.ps
 */
std::string ToDimacsFile(Solver &, fs::path);

} // namespace encoder
} // namespace solver
