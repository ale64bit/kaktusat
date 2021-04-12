#include "solver/encoder/coloring.h"

#include <set>

#include "solver/encoder/cardinality.h"
#include "util/log.h"

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

GraphEdges FlowerSnark(int q) {
  CHECK(q > 0) << "order (q) must be positive, got q=" << q;
  GraphEdges edges;
  for (int j = 1; j <= q; ++j) {
    edges.emplace_back("t" + std::to_string(j),
                       "t" + std::to_string((j % q) + 1));
    edges.emplace_back("t" + std::to_string(j), "u" + std::to_string(j));
    edges.emplace_back("u" + std::to_string(j), "v" + std::to_string(j));
    edges.emplace_back("u" + std::to_string(j), "w" + std::to_string(j));
    edges.emplace_back("v" + std::to_string(j),
                       "w" + std::to_string((j % q) + 1));
    edges.emplace_back("w" + std::to_string(j),
                       "v" + std::to_string((j % q) + 1));
  }
  return edges;
}

GraphEdges FlowerSnarkLine(int q) {
  CHECK(q > 0) << "order size (q) must be positive, got q=" << q;
  GraphEdges edges;
  for (int j = 1; j <= q; ++j) {
    edges.emplace_back("a" + std::to_string(j),
                       "a" + std::to_string((j % q) + 1));
    edges.emplace_back("a" + std::to_string(j), "b" + std::to_string(j));
    edges.emplace_back("a" + std::to_string(j),
                       "b" + std::to_string((j % q) + 1));
    edges.emplace_back("b" + std::to_string(j), "c" + std::to_string(j));
    edges.emplace_back("b" + std::to_string(j), "d" + std::to_string(j));
    edges.emplace_back("c" + std::to_string(j), "d" + std::to_string(j));
    edges.emplace_back("c" + std::to_string(j), "e" + std::to_string(j));
    edges.emplace_back("d" + std::to_string(j), "f" + std::to_string(j));
    edges.emplace_back("e" + std::to_string(j),
                       "d" + std::to_string((j % q) + 1));
    edges.emplace_back("f" + std::to_string(j),
                       "c" + std::to_string((j % q) + 1));
    edges.emplace_back("e" + std::to_string(j),
                       "f" + std::to_string((j % q) + 1));
    edges.emplace_back("f" + std::to_string(j),
                       "e" + std::to_string((j % q) + 1));
  }
  return edges;
}

} // namespace graph

} // namespace encoder
} // namespace solver
