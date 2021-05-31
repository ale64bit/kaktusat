#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "repl/context.h"
#include "repl/result.h"
#include "repl/token.h"
#include "solver/solver.h"

class Context;

using Lookahead = std::vector<Token>::const_iterator;

struct Bindings {
  std::set<std::string> variables;
  std::set<std::string> formulas;
};

class Expr {
public:
  enum class Tag {
    kConst,
    kVariableID,
    kFormulaID,
    kNeg,
    kBin,
    kSub,
  };

  virtual ~Expr() {}
  virtual std::unique_ptr<Expr> Copy() const = 0;
  virtual Bindings CollectBindings() const = 0;
  virtual Result<Expr> Eval(Context &) const = 0;
  virtual Tag GetTag() const = 0;
  virtual int Size() const = 0;
  virtual int Depth() const = 0;
  virtual solver::Var ToSolver(solver::Solver &) const = 0;
  virtual std::string ToString() const = 0;

  static Result<Expr> Parse(Lookahead &);
};

class ConstExpr : public Expr {
public:
  ConstExpr(bool);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kConst; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

  bool GetValue() const { return value_; }

private:
  const bool value_;
};

class VariableIDExpr : public Expr {
public:
  VariableIDExpr(std::string id);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kVariableID; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

  std::string GetID() const { return id_; }

private:
  std::string id_;
};

class FormulaIDExpr : public Expr {
public:
  FormulaIDExpr(std::string id);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kFormulaID; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

  std::string GetID() const { return id_; }

private:
  std::string id_;
};

class NegExpr : public Expr {
public:
  NegExpr(std::unique_ptr<Expr> rhs);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kNeg; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

private:
  std::unique_ptr<Expr> rhs_;
};

enum class BinaryConnective {
  kAnd,
  kOr,
  kXor,
  kImpl,
  kEq,
};

class BinExpr : public Expr {
public:
  BinExpr(BinaryConnective conn, std::unique_ptr<Expr> lhs,
          std::unique_ptr<Expr> rhs);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kBin; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

private:
  BinaryConnective conn_;
  std::unique_ptr<Expr> lhs_;
  std::unique_ptr<Expr> rhs_;
};

class SubExpr : public Expr {
public:
  SubExpr(std::string id, std::map<std::string, std::unique_ptr<Expr>> subs);

  std::unique_ptr<Expr> Copy() const override;
  Bindings CollectBindings() const override;
  Result<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kSub; };
  int Size() const override;
  int Depth() const override;
  solver::Var ToSolver(solver::Solver &) const override;
  std::string ToString() const override;

private:
  std::string id_;
  std::map<std::string, std::unique_ptr<Expr>> subs_;
};
