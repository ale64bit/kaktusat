#pragma once

#include "solver/solver.h"

#include <string>

namespace solver {
namespace builder {

std::string FromDimacsFile(Solver &, std::string);

} // namespace builder
} // namespace solver
