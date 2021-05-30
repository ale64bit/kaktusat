#include "repl/expr.h"

#include <cassert>

#define ASSIGN_OR_RETURN(expr, f)                                              \
  do {                                                                         \
    auto _res = (f);                                                           \
    if (!ResultOK(_res)) {                                                     \
      return _res;                                                             \
    }                                                                          \
    assert(std::holds_alternative<std::unique_ptr<Expr>>(_res));               \
    expr = std::move(std::get<std::unique_ptr<Expr>>(_res));                   \
  } while (false)

Result<Expr> ConstExpr::Eval(Context &ctx) const { return this->Copy(); }

Result<Expr> VariableIDExpr::Eval(Context &ctx) const {
  auto img = ctx.SubImg(id_);
  return img == nullptr ? this->Copy() : img->Eval(ctx);
}

Result<Expr> FormulaIDExpr::Eval(Context &ctx) const {
  auto img = ctx.Get(id_);
  return img == nullptr ? this->Copy() : img->Eval(ctx);
}

Result<Expr> NegExpr::Eval(Context &ctx) const {
  std::unique_ptr<Expr> rhs;

  ASSIGN_OR_RETURN(rhs, rhs_->Eval(ctx));
  if (rhs->GetTag() == Expr::Tag::kConst) {
    return std::make_unique<ConstExpr>(
        !static_cast<const ConstExpr &>(*rhs).GetValue());
  }
  return std::make_unique<NegExpr>(std::move(rhs));
}

Result<Expr> BinExpr::Eval(Context &ctx) const {
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;

  ASSIGN_OR_RETURN(lhs, lhs_->Eval(ctx));
  ASSIGN_OR_RETURN(rhs, rhs_->Eval(ctx));

  // Simplify
  auto HasValue = [](const std::unique_ptr<Expr> &e) {
    return e->GetTag() == Expr::Tag::kConst;
  };
  auto GetValue = [&HasValue](const std::unique_ptr<Expr> &e) {
    assert(HasValue(e));
    return static_cast<const ConstExpr &>(*e).GetValue();
  };
  auto IsFalse = [&HasValue, &GetValue](const std::unique_ptr<Expr> &e) {
    return HasValue(e) && !GetValue(e);
  };
  auto IsTrue = [&HasValue, &GetValue](const std::unique_ptr<Expr> &e) {
    return HasValue(e) && GetValue(e);
  };

  switch (conn_) {
  case BinaryConnective::kAnd:
    if (HasValue(lhs) && HasValue(rhs)) {
      return std::make_unique<ConstExpr>(GetValue(lhs) && GetValue(rhs));
    } else if (IsTrue(lhs)) {
      return rhs;
    } else if (IsTrue(rhs)) {
      return lhs;
    }
    break;
  case BinaryConnective::kOr:
    if (HasValue(lhs) && HasValue(rhs)) {
      return std::make_unique<ConstExpr>(GetValue(lhs) || GetValue(rhs));
    } else if (IsFalse(lhs)) {
      return rhs;
    } else if (IsFalse(rhs)) {
      return lhs;
    }
    break;
  case BinaryConnective::kXor:
    if (HasValue(lhs) && HasValue(rhs)) {
      return std::make_unique<ConstExpr>(GetValue(lhs) != GetValue(rhs));
    }
    break;
  case BinaryConnective::kImpl:
    if (HasValue(lhs) && HasValue(rhs)) {
      return std::make_unique<ConstExpr>(!GetValue(lhs) || GetValue(rhs));
    }
    break;
  case BinaryConnective::kEq:
    if (HasValue(lhs) && HasValue(rhs)) {
      return std::make_unique<ConstExpr>((GetValue(lhs) && GetValue(rhs)) ||
                                         (!GetValue(rhs) && !GetValue(lhs)));
    }
    break;
  }

  return std::make_unique<BinExpr>(conn_, std::move(lhs), std::move(rhs));
}

Result<Expr> SubExpr::Eval(Context &ctx) const {
  auto f = ctx.Get(id_);
  if (!f) {
    return std::make_unique<FormulaIDExpr>(id_);
  }
  ctx.PushSub(subs_);
  ASSIGN_OR_RETURN(f, f->Eval(ctx));
  ctx.PopSub();
  return f;
}
