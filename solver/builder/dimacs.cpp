#include "solver/builder/dimacs.h"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace solver {
namespace builder {

constexpr size_t kMaxLineLen = 1024;
constexpr size_t kMaxFormatLen = 1024;

std::string FromDimacsFile(Solver &solver, std::string filename) {
  std::unique_ptr<std::FILE, decltype(&std::fclose)> f(
      std::fopen(filename.c_str(), "r"), &std::fclose);
  if (!f) {
    return "failed to open file";
  }

  int n = 0;
  int m = 0;
  char line[kMaxLineLen + 1];
  char format[kMaxFormatLen + 1];
  while (std::fgets(line, kMaxLineLen, f.get()) == line) {
    switch (line[0]) {
    case 'c':
      break;
    case 'p':
      if (std::sscanf(line + 1, " %s %d %d", format, &n, &m) != 3) {
        return "invalid problem line '" + std::string(line) + " '";
      }
      if (std::strcmp(format, "cnf") != 0) {
        return "invalid format '" + std::string(format) +
               "'. Only 'cnf' is supported.";
      }
      assert(n > 0);
      assert(m > 0);
      goto read_clauses;
      break;
    default:
      return "unexpected line '" + std::string(line) + "'";
    }
  }

read_clauses:
  for (int i = 0; i < m; ++i) {
    std::vector<Lit> clause;
    int lit = 0;
    while (std::fscanf(f.get(), "%d", &lit) == 1) {
      if (lit == 0)
        break;
      Var x = solver.NewOrGetVar(std::to_string(abs(lit)));
      clause.push_back(lit > 0 ? x : ~x);
    }
    solver.AddClause(clause);
    clause.clear();
  }

  return "";
}

} // namespace builder
} // namespace solver
