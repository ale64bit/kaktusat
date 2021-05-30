#pragma once

#include <vector>

#include "repl/context.h"
#include "repl/expr.h"
#include "repl/parsing.h"
#include "repl/result.h"
#include "repl/token.h"

using Lookahead = std::vector<Token>::const_iterator;

class Cmd {
public:
  virtual ~Cmd() {}
  virtual void Eval(Context &) const = 0;

  static Result<Cmd> Parse(Lookahead &);
};

class LetCmd : public Cmd {
public:
  LetCmd(std::string id, std::unique_ptr<Expr> expr);

  void Eval(Context &) const override;

private:
  std::string id_;
  std::unique_ptr<Expr> expr_;
};

class NopCmd : public Cmd {
public:
  NopCmd(std::unique_ptr<Expr> expr);

  void Eval(Context &) const override;

private:
  std::unique_ptr<Expr> expr_;
};

class SizeCmd : public Cmd {
public:
  SizeCmd(std::unique_ptr<Expr> expr);

  void Eval(Context &) const override;

private:
  std::unique_ptr<Expr> expr_;
};

class DepthCmd : public Cmd {
public:
  DepthCmd(std::unique_ptr<Expr> expr);

  void Eval(Context &) const override;

private:
  std::unique_ptr<Expr> expr_;
};

class TTCmd : public Cmd {
public:
  TTCmd(std::unique_ptr<Expr> expr);

  void Eval(Context &) const override;

private:
  std::unique_ptr<Expr> expr_;
};
