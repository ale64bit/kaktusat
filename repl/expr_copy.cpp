#include "repl/expr.h"

std::unique_ptr<Expr> ConstExpr::Copy() const {
  return std::make_unique<ConstExpr>(value_);
}

std::unique_ptr<Expr> VariableIDExpr::Copy() const {
  return std::make_unique<VariableIDExpr>(id_);
}

std::unique_ptr<Expr> FormulaIDExpr::Copy() const {
  return std::make_unique<FormulaIDExpr>(id_);
}

std::unique_ptr<Expr> NegExpr::Copy() const {
  return std::make_unique<NegExpr>(rhs_->Copy());
}

std::unique_ptr<Expr> BinExpr::Copy() const {
  return std::make_unique<BinExpr>(conn_, lhs_->Copy(), rhs_->Copy());
}

std::unique_ptr<Expr> SubExpr::Copy() const {
  std::map<std::string, std::unique_ptr<Expr>> subs;
  for (const auto &[k, v] : subs_) {
    subs.emplace(k, v->Copy());
  }
  return std::make_unique<SubExpr>(id_, std::move(subs));
}

std::unique_ptr<Expr> LetExpr::Copy() const {
  return std::make_unique<LetExpr>(id_, expr_->Copy());
}
