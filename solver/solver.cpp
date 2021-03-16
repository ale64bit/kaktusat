#include "solver.h"

#include <cassert>
#include <set>
#include <sstream>

namespace solver {

Var::Var(int x) : x(x) { assert(x != 0); }
bool Var::operator==(const Var &that) const { return this->x == that.x; }
bool Var::operator!=(const Var &that) const { return this->x != that.x; }
bool Var::operator<(const Var &that) const { return this->x < that.x; }
Var::operator Lit() const { return Lit(2 * x); }
Lit Var::operator~() const { return Lit(2 * x + 1); }

Lit::Lit(int l) : l(l) { assert(l != 0); }
bool Lit::operator==(const Lit &that) const { return this->l == that.l; }
bool Lit::operator!=(const Lit &that) const { return this->l != that.l; }
bool Lit::operator<(const Lit &that) const { return this->l < that.l; }
Lit Lit::operator~() const { return Lit(l ^ 1); }
Var Lit::V() const { return Var(l >> 1); }

Solver::Solver() : n_(0) {}

Var Solver::NewVar(std::string name) {
  assert(nameToVar_.count(name) == 0);
  name_.push_back(name);
  ++n_;
  Var x(n_);
  nameToVar_.emplace(name, x);
  return x;
}

Var Solver::NewOrGetVar(std::string name) {
  if (nameToVar_.count(name)) {
    return nameToVar_.at(name);
  } else {
    return NewVar(name);
  }
}

Var Solver::GetVar(std::string name) const {
  assert(nameToVar_.count(name) > 0);
  return nameToVar_.at(name);
}

std::string Solver::NameOf(Var x) const { return name_[x.ID() - 1]; }

void Solver::AddClause(std::vector<Lit> c) { clauses_.emplace_back(c); }

bool Solver::Verify(const std::vector<Lit> &solution,
                    std::string *errMsg) const {
  std::vector<bool> used(2 * n_ + 2, false);
  for (auto lit : solution) {
    if (used[lit.ID()] || used[(~lit).ID()]) {
      if (errMsg) {
        *errMsg = "literal used multiple times";
      }
      return false;
    }
    used[lit.ID()] = true;
  }
  for (auto clause : clauses_) {
    bool ok = false;
    for (auto lit : clause) {
      if (used[lit.ID()]) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      if (errMsg) {
        *errMsg = "clause left unsatisfied";
      }
      return false;
    }
  }
  return true;
}

std::string Solver::ToString(Var x) const { return name_[x.ID() - 1]; }

std::string Solver::ToString(Lit l) const {
  std::string s = NameOf(l.V());
  return (l != l.V()) ? "¬" + s : s;
}

std::string Solver::ToString(const std::vector<Lit> &lits, bool raw) const {
  std::stringstream out;
  bool first = true;
  for (auto l : lits) {
    if (!first) {
      out << ", ";
    }
    first = false;
    if (raw) {
      out << ((l.ID() & 1) ? "-" : "") << (l.ID() >> 1);
    } else {
      out << ToString(l);
    }
  }
  return out.str();
}

std::string Solver::ToString() const {
  std::stringstream out;
  bool first = true;
  for (auto c : clauses_) {
    if (!first) {
      out << " ∧ ";
    }
    first = false;
    out << "(";
    for (size_t i = 0; i < c.size(); ++i) {
      if (i > 0) {
        out << " ∨ ";
      }
      out << ToString(c[i]);
    }
    out << ")";
  }
  return out.str();
}

} // namespace solver
