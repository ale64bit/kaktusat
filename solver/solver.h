#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "util/check.h"

namespace solver {

class Var;
class Lit;

class Var {
public:
  Var(int x);

  bool operator==(const Var &that) const;
  bool operator!=(const Var &that) const;
  bool operator<(const Var &that) const;
  Lit operator~() const;
  operator Lit() const;
  int ID() const { return x; }

private:
  int x;
};

class Lit {
public:
  Lit(int l);

  bool operator==(const Lit &that) const;
  bool operator!=(const Lit &that) const;
  bool operator<(const Lit &that) const;
  Lit operator~() const;
  Var V() const;
  int ID() const { return l; }
  bool IsNeg() const { return (l & 1); }

private:
  int l;
};

enum class Result {
  kSAT,
  kUNSAT,
  kUnknown,
};

class Solver {
public:
  Solver();
  virtual ~Solver() {}

  Var NewVar(std::string);
  Var NewTempVar(std::string = "t");
  Var NewOrGetVar(std::string);
  Var GetVar(std::string) const;
  std::string NameOf(Var) const;
  void AddClause(std::vector<Lit>);
  int NumVars() const { return n_; }
  int NumClauses() const { return static_cast<int>(clauses_.size()); }
  std::vector<std::string> GetVars() const;
  std::vector<std::vector<Lit>> GetClauses() const;
  void Reset();
  bool Verify(const std::vector<Lit> &, std::string * = nullptr) const;
  std::string ToString(Var) const;
  std::string ToString(Lit) const;
  std::string ToString(const std::vector<Lit> &lits, bool = false) const;
  std::string ToString() const;

  virtual std::pair<Result, std::vector<Lit>> Solve() = 0;
  virtual std::string ID() const = 0;

protected:
  int n_;
  std::vector<std::string> name_;
  std::vector<std::vector<Lit>> clauses_;
  std::unordered_map<std::string, Var> nameToVar_;
  int tmpID_;
};

} // namespace solver
