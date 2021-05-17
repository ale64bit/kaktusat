#include "repl/expr.h"

std::unique_ptr<Expr> ConstExpr::Eval(Context &ctx) const {
  return this->Copy();
}

std::unique_ptr<Expr> VariableIDExpr::Eval(Context &ctx) const {
  auto img = ctx.SubImg(id_);
  return img == nullptr ? this->Copy() : img->Eval(ctx);
}

std::unique_ptr<Expr> FormulaIDExpr::Eval(Context &ctx) const {
  auto img = ctx.Get(id_);
  return img == nullptr ? this->Copy() : img->Eval(ctx);
}

std::unique_ptr<Expr> NegExpr::Eval(Context &ctx) const {
  auto res = rhs_->Eval(ctx);
  if (res->GetTag() == Expr::Tag::kConst) {
    return std::make_unique<ConstExpr>(
        !static_cast<const ConstExpr &>(*res).GetValue());
  }
  return std::make_unique<NegExpr>(std::move(res));
}

std::unique_ptr<Expr> BinExpr::Eval(Context &ctx) const {
  auto lhs = lhs_->Eval(ctx);
  auto rhs = rhs_->Eval(ctx);

  // Simplify
  auto IsTrue = [](const std::unique_ptr<Expr> &e) {
    return e->GetTag() == Expr::Tag::kConst &&
           static_cast<const ConstExpr &>(*e).GetValue();
  };
  auto IsFalse = [](const std::unique_ptr<Expr> &e) {
    return e->GetTag() == Expr::Tag::kConst &&
           !static_cast<const ConstExpr &>(*e).GetValue();
  };

  switch (conn_) {
  case BinaryConnective::kAnd:
    if (IsFalse(lhs) || IsFalse(rhs)) {
      return std::make_unique<ConstExpr>(false);
    } else if (IsTrue(lhs)) {
      return rhs;
    } else if (IsTrue(rhs)) {
      return lhs;
    }
    break;
  case BinaryConnective::kOr:
    if (IsTrue(lhs) || IsTrue(rhs)) {
      return std::make_unique<ConstExpr>(true);
    } else if (IsFalse(lhs)) {
      return rhs;
    } else if (IsFalse(rhs)) {
      return lhs;
    }
    break;
  case BinaryConnective::kXor:
    // TODO implement
    break;
  case BinaryConnective::kImpl:
    // TODO implement
    break;
  case BinaryConnective::kEq:
    // TODO implement
    break;
  }

  return std::make_unique<BinExpr>(conn_, std::move(lhs), std::move(rhs));
}

std::unique_ptr<Expr> SubExpr::Eval(Context &ctx) const {
  auto f = ctx.Get(id_);
  if (!f) {
    return std::make_unique<FormulaIDExpr>(id_);
  }
  ctx.PushSub(subs_);
  f = f->Eval(ctx);
  ctx.PopSub();
  return f;
}

std::unique_ptr<Expr> LetExpr::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  ctx.Set(id_, res->Copy());
  return res;
}
