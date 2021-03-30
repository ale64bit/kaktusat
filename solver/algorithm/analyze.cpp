#include "solver/algorithm/analyze.h"

#include <iomanip>
#include <map>
#include <set>

#include "util/log.h"

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

  LOG << "N=" << NumVars() << " M=" << NumClauses();

  LOG << "Clause-to-variable ratio: " << std::fixed << std::setprecision(3)
      << static_cast<double>(NumClauses()) / NumVars();

  LOG << "Pure literals: ";
  for (int i = 1; i <= NumVars(); ++i) {
    Var x(i);
    if (litSeen.count(x) && !litSeen.count(~x)) {
      LOG << ToString(x) << ' ';
    } else if (!litSeen.count(x) && litSeen.count(~x)) {
      LOG << ToString(~x) << ' ';
    }
  }

  if (clauseLenCnt.count(0)) {
    LOG << "Instance contains empty clause, so it's trivially unsatisfiable";
  }

  if (!repeated.empty()) {
    LOG << "Clauses with repeated literals:";
    for (const auto &[i, clause] : repeated) {
      LOG << "\t" << i << ": " << ToString(clause);
    }
  }

  if (!tautologies.empty()) {
    LOG << "Tautologies:";
    for (const auto &[i, clause] : tautologies) {
      LOG << "\t" << i << ": " << ToString(clause);
    }
  }

  LOG << "Clause length counts:";
  for (const auto &[len, cnt] : clauseLenCnt) {
    LOG << "\t" << len << ": " << cnt;
  }

  return {Result::kUnknown, {}};
}

} // namespace algorithm
} // namespace solver
