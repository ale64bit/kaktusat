#include "solver/builder/coloring.h"

#include <set>

#include "solver/builder/cardinality.h"

namespace solver {
namespace builder {

void Coloring(Solver &solver, int d,
              std::vector<std::pair<std::string, std::string>> edges,
              Mode mode) {
  // Collect the unique vertices.
  std::set<std::string> vertices;
  for (const auto &[u, v] : edges) {
    vertices.insert(u);
    vertices.insert(v);
  }

  for (const auto &u : vertices) {
    std::vector<Lit> lu;
    for (int j = 0; j < d; ++j) {
      lu.push_back(solver.NewVar(u + "_" + std::to_string(j)));
    }
    // u must have exactly one color.
    ExactlyOne(solver, lu, mode);
  }

  for (const auto &[u, v] : edges) {
    // u and v cannot have the same color.
    for (int j = 0; j < d; ++j) {
      auto x = solver.GetVar(u + "_" + std::to_string(j));
      auto y = solver.GetVar(v + "_" + std::to_string(j));
      solver.AddClause({~x, ~y});
    }
  }
}

} // namespace builder
} // namespace solver
