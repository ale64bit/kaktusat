#include "solver/encoder/dimacs.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "util/log.h"

namespace solver {
namespace encoder {

std::string FromDimacsFile(Solver &solver, fs::path path) {
  std::error_code ec;
  if (!fs::exists(path, ec)) {
    return "file " + path.string() + " does not exist";
  }
  std::ifstream in(path);
  if (!in.is_open()) {
    return "failed to open " + path.string();
  }

  int n = 0;
  int m = 0;
  std::string line;
  std::string format;
  for (std::string line; std::getline(in, line);) {
    std::stringstream lin(line);
    char typ;
    lin >> typ;
    switch (typ) {
    case 'c':
      break;
    case 'p':
      lin >> format >> n >> m;
      if (format != "cnf") {
        return "invalid format '" + std::string(format) +
               "'. Only 'cnf' is supported.";
      }
      if (n <= 0) {
        return "invalid number of variables: n=" + std::to_string(n);
      }
      if (m < 0) {
        return "invalid number of clauses: m=" + std::to_string(m);
      }
      break;
    default:
      return "unexpected line '" + line + "'";
    }
    if (!format.empty()) {
      break;
    }
  }

read_clauses:
  for (int i = 0; i < m; ++i) {
    std::vector<Lit> clause;
    for (int lit = 0; in >> lit;) {
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

std::string ToDimacsFile(Solver &solver, fs::path path) {
  std::ofstream out(path);
  if (!out.is_open()) {
    return "failed to create " + path.string();
  }
  out << "c Variable names:\n";
  for (const auto &name : solver.GetVarNames()) {
    Var x = solver.GetVar(name);
    out << "c \t" << std::setw(9) << x.ID() << " " << std::setw(18) << name
        << " temp=" << solver.IsTemp(x) << '\n';
  }
  out << "p cnf " << solver.NumVars() << " " << solver.NumClauses() << '\n';
  for (const auto &c : solver.GetClauses()) {
    for (const auto &l : c) {
      out << (l.IsPos() ? l.V().ID() : -l.V().ID()) << ' ';
    }
    out << "0\n";
  }
  return "";
}

} // namespace encoder
} // namespace solver
