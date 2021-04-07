#include "solver.h"

#include <set>
#include <sstream>

namespace solver {

// A prefix used for temporary variables, which can't be used in ordinary
// variables.
static constexpr char kTmpSep = '$';

Var::Var(int x) : x(x) {
  CHECK(x > 0) << "variable representation must be positive, got x=" << x;
}
bool Var::operator==(const Var &that) const { return this->x == that.x; }
bool Var::operator!=(const Var &that) const { return this->x != that.x; }
bool Var::operator<(const Var &that) const { return this->x < that.x; }
Var::operator Lit() const { return Lit(2 * x); }
Lit Var::operator~() const { return Lit(2 * x + 1); }

Lit::Lit(int l) : l(l) {
  CHECK(l > 0) << "literal representation must be positive, got l=" << l;
}
bool Lit::operator==(const Lit &that) const { return this->l == that.l; }
bool Lit::operator!=(const Lit &that) const { return this->l != that.l; }
bool Lit::operator<(const Lit &that) const { return this->l < that.l; }
Lit Lit::operator~() const { return Lit(l ^ 1); }
Var Lit::V() const { return Var(l >> 1); }

Solver::Solver() : n_(0), tmpID_(0) {}

Var Solver::NewTempVar(std::string prefix) {
  CHECK(prefix.find(kTmpSep) == std::string::npos)
      << "variable prefixes are not allowed to contain '" << kTmpSep << "'";
  while (nameToVar_.count(prefix + kTmpSep + std::to_string(tmpID_))) {
    ++tmpID_;
  }
  std::string name = prefix + kTmpSep + std::to_string(tmpID_++);
  name_.push_back(name);
  ++n_;
  Var x(n_);
  nameToVar_.emplace(name, x);
  isTemp_.emplace_back(true);
  return x;
}

Var Solver::NewVar(std::string name) {
  CHECK(name.find(kTmpSep) == std::string::npos)
      << "variable names are not allowed to contain '" << kTmpSep << "'";
  CHECK(!name.empty()) << "variable name cannot be empty";
  CHECK(nameToVar_.count(name) == 0)
      << "duplicate variable name '" << name << "'";
  name_.push_back(name);
  ++n_;
  Var x(n_);
  nameToVar_.emplace(name, x);
  isTemp_.emplace_back(false);
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
  CHECK(nameToVar_.count(name) > 0) << "unknown variable name '" << name << "'";
  return nameToVar_.at(name);
}

const std::vector<std::string> &Solver::GetVarNames() const { return name_; }

const std::vector<std::vector<Lit>> &Solver::GetClauses() const {
  return clauses_;
}

std::string Solver::NameOf(Var x) const { return name_[x.ID() - 1]; }

bool Solver::IsTemp(Var x) const { return isTemp_[x.ID() - 1]; }

void Solver::AddClause(std::vector<Lit> c) { clauses_.emplace_back(c); }

void Solver::Reset() {
  n_ = 0;
  name_.clear();
  clauses_.clear();
  nameToVar_.clear();
  tmpID_ = 0;
}

bool Solver::Verify(const std::vector<Lit> &solution,
                    std::string *errMsg) const {
  std::vector<bool> used(2 * n_ + 2, false);
  for (auto lit : solution) {
    if (used[lit.ID()] || used[(~lit).ID()]) {
      if (errMsg) {
        *errMsg = "literal used multiple times: " + ToString(lit.V());
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
        *errMsg = "clause left unsatisfied: " + ToString(clause);
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

std::string Solver::ToString(const std::vector<Lit> &lits, std::string sep,
                             bool raw) const {
  std::stringstream out;
  bool first = true;
  for (auto l : lits) {
    if (!first) {
      out << sep;
    }
    first = false;
    if (raw) {
      out << (l.IsNeg() ? "-" : "") << l.V().ID();
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
