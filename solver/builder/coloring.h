#pragma once

#include <string>
#include <vector>

#include "solver/builder/builder.h"
#include "solver/solver.h"

namespace solver {
namespace builder {

/*
 * Instance builder for graph coloring problems.
 *
 * The resulting instance is satisfiable iff. the underlying graph can be
 * colored using at most d colors. The actual coloring is encoded in the
 * solution found.
 *
 * Resets solver: NO
 *
 * @see: https://en.wikipedia.org/wiki/Graph_coloring
 * @see: 7.2.2.2 - (11), p6
 */
void Coloring(Solver &, int d,
              std::vector<std::pair<std::string, std::string>> edges,
              Mode = Mode::kLessVariables);

} // namespace builder
} // namespace solver
