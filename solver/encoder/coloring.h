#pragma once

#include <string>
#include <vector>

#include "solver/encoder/encoder.h"
#include "solver/solver.h"

namespace solver {
namespace encoder {

/* Represents a graph as a list of edges between named vertices. */
using GraphEdges = std::vector<std::pair<std::string, std::string>>;

/*
 * Instance encoder for graph coloring problems.
 *
 * The resulting instance is satisfiable iff. the underlying graph can be
 * colored using at most d colors. The actual coloring is encoded in the
 * solution found.
 *
 * @see: https://en.wikipedia.org/wiki/Graph_coloring
 * @see: 7.2.2.2 - (11), p6
 */
void Coloring(Solver &, int d, GraphEdges edges, Mode = Mode::kLessVariables);

namespace graph {
/* Some predefined small graph examples. */
GraphEdges Petersen();
GraphEdges McGregor3();
} // namespace graph

} // namespace encoder
} // namespace solver
