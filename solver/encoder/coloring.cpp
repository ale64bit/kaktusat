#include "solver/encoder/coloring.h"

#include <set>

#include "solver/encoder/cardinality.h"

namespace solver {
namespace encoder {

void Coloring(Solver &solver, int d, GraphEdges edges, Mode mode) {
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

namespace graph {

GraphEdges Petersen() {
  return {
      {"1", "2"}, {"1", "5"}, {"1", "6"}, {"2", "3"},  {"2", "7"},
      {"3", "4"}, {"3", "8"}, {"4", "5"}, {"4", "9"},  {"5", "10"},
      {"6", "8"}, {"6", "9"}, {"7", "9"}, {"7", "10"}, {"8", "10"},
  };
}

GraphEdges McGregor3() {
  return {
      {"00", "32"}, {"00", "01"}, {"00", "11"}, {"00", "30"}, {"00", "10"},
      {"01", "00"}, {"01", "32"}, {"01", "02"}, {"01", "12"}, {"01", "11"},
      {"02", "01"}, {"02", "32"}, {"02", "12"}, {"11", "00"}, {"11", "01"},
      {"11", "12"}, {"11", "22"}, {"11", "20"}, {"11", "30"}, {"12", "01"},
      {"12", "02"}, {"12", "32"}, {"12", "21"}, {"12", "22"}, {"12", "11"},
      {"22", "11"}, {"22", "12"}, {"22", "21"}, {"22", "20"}, {"20", "30"},
      {"20", "11"}, {"20", "22"}, {"20", "21"}, {"20", "31"}, {"21", "22"},
      {"21", "12"}, {"21", "32"}, {"21", "31"}, {"21", "20"}, {"30", "00"},
      {"30", "11"}, {"30", "20"}, {"30", "31"}, {"31", "20"}, {"31", "21"},
      {"31", "32"}, {"31", "10"}, {"31", "00"}, {"31", "30"}, {"32", "10"},
      {"32", "31"}, {"32", "21"}, {"32", "12"}, {"32", "02"}, {"32", "01"},
      {"32", "00"}, {"10", "00"}, {"10", "31"}, {"10", "32"},
  };
}

} // namespace graph

} // namespace encoder
} // namespace solver
