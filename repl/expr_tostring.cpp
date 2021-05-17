#include "repl/expr.h"

#include <sstream>

static std::string BinaryConnectiveToString(BinaryConnective conn) {
  switch (conn) {
  case BinaryConnective::kAnd:
    return kConnAndRepr;
  case BinaryConnective::kOr:
    return kConnOrRepr;
  case BinaryConnective::kXor:
    return kConnXorRepr;
  case BinaryConnective::kImpl:
    return kConnImplRepr;
  case BinaryConnective::kEq:
    return kConnEqRepr;
  }
}

std::string ConstExpr::ToString() const {
  return value_ ? kConstTrueRepr : kConstFalseRepr;
}

std::string VariableIDExpr::ToString() const { return id_; }

std::string FormulaIDExpr::ToString() const { return id_; }

std::string NegExpr::ToString() const {
  return "(" + (kConnNotRepr + rhs_->ToString()) + ")";
}

std::string BinExpr::ToString() const {
  return "(" + lhs_->ToString() + " " + BinaryConnectiveToString(conn_) + " " +
         rhs_->ToString() + ")";
}
std::string SubExpr::ToString() const {
  std::stringstream out;
  out << id_ << "(";
  bool first = true;
  for (const auto &[var, expr] : subs_) {
    if (!first) {
      out << ", ";
    }
    first = false;
    out << var << "/" << expr->ToString();
  }
  out << ")";
  return out.str();
}

std::string LetExpr::ToString() const {
  return "let " + id_ + " = " + expr_->ToString();
}
