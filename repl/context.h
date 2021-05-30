#pragma once

#include <map>
#include <memory>
#include <stack>
#include <string>

#include "repl/expr.h"

class Expr;

class Context {
public:
  using Substitution = std::map<std::string, std::unique_ptr<Expr>>;

  Context();
  bool Exists(std::string) const;
  std::unique_ptr<Expr> Get(std::string) const;
  std::unique_ptr<Expr> SubImg(std::string) const;
  void Set(std::string, std::unique_ptr<Expr>);
  void PushSub(const Substitution &);
  void PopSub();

private:
  std::map<std::string, std::unique_ptr<Expr>> bindings_;
  std::stack<Substitution> subStack_;
};
