#pragma once

#include <string>

#include "solver/solver.h"

namespace solver {
namespace builder {

std::string FromDimacsFile(Solver &, std::string);

} // namespace builder
} // namespace solver
