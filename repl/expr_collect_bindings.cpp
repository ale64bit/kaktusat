
#include "repl/expr.h"

static void Merge(Bindings &dst, Bindings &src) {
  if (dst.variables.size() < src.variables.size()) {
    std::swap(dst.variables, src.variables);
  }
  if (dst.formulas.size() < src.formulas.size()) {
    std::swap(dst.formulas, src.formulas);
  }
  for (const auto &id : src.variables) {
    dst.variables.insert(id);
  }
  for (const auto &id : src.formulas) {
    dst.formulas.insert(id);
  }
}

Bindings ConstExpr::CollectBindings() const { return Bindings{{}, {}}; }

Bindings VariableIDExpr::CollectBindings() const { return Bindings{{id_}, {}}; }

Bindings FormulaIDExpr::CollectBindings() const { return Bindings{{}, {id_}}; }

Bindings NegExpr::CollectBindings() const { return rhs_->CollectBindings(); }

Bindings BinExpr::CollectBindings() const {
  auto lhs = lhs_->CollectBindings();
  auto rhs = rhs_->CollectBindings();
  Merge(lhs, rhs);
  return lhs;
}

Bindings SubExpr::CollectBindings() const {
  Bindings ret{{}, {id_}};
  for (const auto &[k, v] : subs_) {
    auto b = v->CollectBindings();
    Merge(ret, b);
  }
  return ret;
}
