#include "solver/builder/cardinality.h"

namespace solver {
namespace builder {

void ExactlyOne(Solver &solver, const std::vector<Lit> &y, Mode mode) {
  AtLeastOne(solver, y, mode);
  AtMostOne(solver, y, mode);
}

void AtLeastOne(Solver &solver, const std::vector<Lit> &y, Mode) {
  solver.AddClause(y);
}

/*
 * Simple implementation: for each pair x and y, one of them must be false.
 *
 * It doesn't use new variables, but generates O(|y|^2) binary clauses.
 *
 * @see: 7.2.2.2 - p6
 */
static void AtMostOneSimple(Solver &solver, const std::vector<Lit> &y) {
  for (size_t i = 0; i < y.size(); ++i) {
    for (size_t j = i + 1; j < y.size(); ++j) {
      solver.AddClause({~y[i], ~y[j]});
    }
  }
}

/*
 * Generates less clauses, at the expense of more variables.
 *
 * Produces floor((p-3)/2) new variables and 3*p-5 clauses, where p=|y|.
 *
 * @see: 7.2.2.2 - p6
 * @see: 7.2.2.2 - exercise 12, p134
 */
static void AtMostOneLessClauses(Solver &solver, const std::vector<Lit> &y) {
  if (y.size() <= 4) {
    AtMostOneSimple(solver, y);
    return;
  }
  auto t1 = solver.NewTempVar();
  AtMostOneSimple(solver, {y[0], y[1], y[2], t1});
  for (size_t i = 3; i < y.size(); i += 2) {
    if (i == y.size() - 1) {
      // 1 remaining
      AtMostOneSimple(solver, {~t1, y[i]});
      break;
    } else if (i == y.size() - 2) {
      // 2 remaining
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1]});
      break;
    } else if (i == y.size() - 3) {
      // 3 remaining
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1], y[i + 2]});
      break;
    } else {
      // >3 remaining
      auto t2 = solver.NewTempVar();
      AtMostOneSimple(solver, {~t1, y[i], y[i + 1], t2});
      t1 = t2;
    }
  }
}

void AtMostOne(Solver &solver, const std::vector<Lit> &y, Mode mode) {
  switch (mode) {
  case Mode::kLessVariables:
    AtMostOneSimple(solver, y);
    break;
  case Mode::kLessClauses:
    AtMostOneLessClauses(solver, y);
    break;
  }
}

} // namespace builder
} // namespace solver
