#include "solver/algorithm/analyze.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <set>

namespace solver {
namespace algorithm {

std::pair<Result, std::vector<Lit>> Analyze::Solve() {
  std::set<Lit> litSeen;
  std::set<Var> varSeen;
  std::map<int, int> clauseLenCnt;
  std::vector<std::pair<int, std::vector<Lit>>> repeated;
  std::vector<std::pair<int, std::vector<Lit>>> tautologies;
  int clauseIndex = 1;
  for (const auto &clause : clauses_) {
    clauseLenCnt[clause.size()]++;
    for (const auto &l : clause) {
      varSeen.insert(l.V());
      litSeen.insert(l);
    }
    bool hasRepeated = false;
    bool isTautological = false;
    for (size_t i = 0; i < clause.size(); ++i) {
      for (size_t j = i + 1; j < clause.size(); ++j) {
        if (clause[i] == clause[j]) {
          hasRepeated = true;
        }
        if (clause[i] == ~clause[j]) {
          isTautological = true;
        }
      }
    }
    if (hasRepeated) {
      repeated.emplace_back(clauseIndex, clause);
    }
    if (isTautological) {
      tautologies.emplace_back(clauseIndex, clause);
    }
    ++clauseIndex;
  }

  std::clog << "N=" << NumVars() << " M=" << NumClauses() << '\n';

  std::clog << "Clause-to-variable ratio: " << std::fixed
            << std::setprecision(3)
            << static_cast<double>(NumClauses()) / NumVars() << '\n';

  std::clog << "Pure literals: ";
  for (int i = 1; i <= NumVars(); ++i) {
    Var x(i);
    if (litSeen.count(x) && !litSeen.count(~x)) {
      std::clog << ToString(x) << ' ';
    } else if (!litSeen.count(x) && litSeen.count(~x)) {
      std::clog << ToString(~x) << ' ';
    }
  }
  std::clog << '\n';

  if (clauseLenCnt.count(0)) {
    std::clog
        << "Instance contains empty clause, so it's trivially unsatisfiable"
        << '\n';
  }

  if (!repeated.empty()) {
    std::clog << "Clauses with repeated literals:\n";
    for (const auto &[i, clause] : repeated) {
      std::clog << "\t" << i << ": " << ToString(clause) << '\n';
    }
  }

  if (!tautologies.empty()) {
    std::clog << "Tautologies:\n";
    for (const auto &[i, clause] : tautologies) {
      std::clog << "\t" << i << ": " << ToString(clause) << '\n';
    }
  }

  std::clog << "Clause length counts:" << '\n';
  for (const auto &[len, cnt] : clauseLenCnt) {
    std::clog << "\t" << len << ": " << cnt << '\n';
  }

  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
