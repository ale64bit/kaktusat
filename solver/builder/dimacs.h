#pragma once

#include "solver/solver.h"

#include <string>

namespace solver {
namespace builder {

std::string FromDimacsFile(std::string, Solver &);

} // namespace builder
} // namespace solver
