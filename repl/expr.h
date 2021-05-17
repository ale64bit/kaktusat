#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "repl/context.h"
#include "repl/token.h"

class Context;

class Expr {
public:
  enum class Tag {
    kConst,
    kVariableID,
    kFormulaID,
    kNeg,
    kBin,
    kSub,
    kLet,
  };

  virtual ~Expr() {}
  virtual std::unique_ptr<Expr> Copy() const = 0;
  virtual std::unique_ptr<Expr> Eval(Context &) const = 0;
  virtual Tag GetTag() const = 0;
  virtual std::string ToString() const = 0;

  static std::unique_ptr<Expr> Parse(const std::vector<Token> &,
                                     std::vector<std::string> &);
};

class ConstExpr : public Expr {
public:
  ConstExpr(bool);
  std::unique_ptr<Expr> Copy() const override;
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kConst; };
  std::string ToString() const override;

  bool GetValue() const { return value_; }

private:
  const bool value_;
};

class VariableIDExpr : public Expr {
public:
  VariableIDExpr(std::string id);
  std::unique_ptr<Expr> Copy() const override;
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kVariableID; };
  std::string ToString() const override;

  std::string GetID() const { return id_; }

private:
  std::string id_;
};

class FormulaIDExpr : public Expr {
public:
  FormulaIDExpr(std::string id);
  std::unique_ptr<Expr> Copy() const override;
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kFormulaID; };
  std::string ToString() const override;

  std::string GetID() const { return id_; }

private:
  std::string id_;
};

class NegExpr : public Expr {
public:
  NegExpr(std::unique_ptr<Expr> rhs);
  std::unique_ptr<Expr> Copy() const override;
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kNeg; };
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
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kBin; };
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
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kSub; };
  std::string ToString() const override;

private:
  std::string id_;
  std::map<std::string, std::unique_ptr<Expr>> subs_;
};

class LetExpr : public Expr {
public:
  LetExpr(std::string id, std::unique_ptr<Expr> expr);
  std::unique_ptr<Expr> Copy() const override;
  std::unique_ptr<Expr> Eval(Context &) const override;
  Tag GetTag() const override { return Tag::kLet; };
  std::string ToString() const override;

private:
  std::string id_;
  std::unique_ptr<Expr> expr_;
};
