#include "solver/builder/cardinality.h"

namespace solver {
namespace builder {

void ExactlyOne(Solver &solver, const std::vector<Lit> &y) {
  AtLeastOne(solver, y);
  AtMostOne(solver, y);
}

void AtLeastOne(Solver &solver, const std::vector<Lit> &y) {
  solver.AddClause(y);
}

void AtMostOneSimple(Solver &solver, const std::vector<Lit> &y) {
  for (size_t i = 0; i < y.size(); ++i) {
    for (size_t j = i + 1; j < y.size(); ++j) {
      solver.AddClause({~y[i], ~y[j]});
    }
  }
}

void AtMostOne(Solver &solver, Lit a, Lit b) { solver.AddClause({~a, ~b}); }

void AtMostOne(Solver &solver, Lit a, Lit b, Lit c) {
  solver.AddClause({~a, ~b});
  solver.AddClause({~a, ~c});
  solver.AddClause({~b, ~c});
};

void AtMostOne(Solver &solver, Lit a, Lit b, Lit c, Lit d) {
  solver.AddClause({~a, ~b});
  solver.AddClause({~a, ~c});
  solver.AddClause({~a, ~d});
  solver.AddClause({~b, ~c});
  solver.AddClause({~b, ~d});
  solver.AddClause({~c, ~d});
};

void AtMostOne(Solver &solver, const std::vector<Lit> &y) {
  if (y.size() <= 4) {
    AtMostOneSimple(solver, y);
    return;
  }
  auto t1 = solver.NewTempVar();
  AtMostOne(solver, y[0], y[1], y[2], t1);
  for (size_t i = 3; i < y.size(); i += 2) {
    if (i == y.size() - 1) {
      // 1 remaining
      AtMostOne(solver, ~t1, y[i]);
      break;
    } else if (i == y.size() - 2) {
      // 2 remaining
      AtMostOne(solver, ~t1, y[i], y[i + 1]);
      break;
    } else if (i == y.size() - 3) {
      // 3 remaining
      AtMostOne(solver, ~t1, y[i], y[i + 1], y[i + 2]);
      break;
    } else {
      // >3 remaining
      auto t2 = solver.NewTempVar();
      AtMostOne(solver, ~t1, y[i], y[i + 1], t2);
      t1 = t2;
    }
  }
}

} // namespace builder
} // namespace solver
