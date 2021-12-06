#include "repl/cmd.h"

#include <algorithm>
#include <cassert>
#include <curses.h>
#include <sstream>

#include "repl/context.h"
#include "solver/algorithm/algorithm.h"

template <typename T> void PrintErrors(const Result<T> &res) {
  auto errors = std::get<std::vector<std::string>>(res);
  assert(!errors.empty());
  for (const auto &err : errors) {
    printw("\terror: %s\n", err.c_str());
  }
}

template <class T> static std::string Join(const T &xs, std::string sep = "") {
  std::stringstream out;
  bool first = true;
  for (const auto &x : xs) {
    if (!first) {
      out << sep;
    }
    first = false;
    out << x;
  }
  return out.str();
}

void LetCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  auto bindings = expr->CollectBindings();
  if (!bindings.formulas.empty()) {
    printw("\terror: expression '%s' contains free formulas: %s\n",
           expr->ToString().c_str(), Join(bindings.formulas, ", ").c_str());
    return;
  }
  assert(bindings.formulas.empty());
  ctx.Set(id_, expr->Copy());
  printw("\t%s(%s) = %s\n", id_.c_str(), Join(bindings.variables, ", ").c_str(),
         expr->ToString().c_str());
}

void NopCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  auto bindings = expr->CollectBindings();
  printw("\t= %s\n", expr->ToString().c_str());
}

void SizeCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  printw("\t= %d\n", expr->Size());
}

void DepthCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  printw("\t= %d\n", expr->Depth());
}

void TTCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  auto bindings = expr->CollectBindings();
  std::vector<std::string> allBindings;
  for (const auto &v : bindings.variables) {
    allBindings.push_back(v);
  }
  for (const auto &f : bindings.formulas) {
    allBindings.push_back(f);
  }

  const size_t n = allBindings.size();

  // Compute column widths.
  std::vector<int> colWidth(n);
  for (size_t i = 0; i < n; ++i) {
    colWidth[i] = 1 + ActualSize(allBindings[i]) + 1;
  }

  // Print header row.
  addch('\t');
  attron(A_ALTCHARSET);
  addch(ACS_ULCORNER);
  for (size_t i = 0; i < n; ++i) {
    for (int j = 0; j < colWidth[i]; ++j) {
      addch(ACS_HLINE);
    }
    addch(ACS_TTEE);
  }
  for (int j = 0; j < 3; ++j) {
    addch(ACS_HLINE);
  }
  addch(ACS_URCORNER);
  attroff(A_ALTCHARSET);
  addch('\n');

  addch('\t');
  attron(A_ALTCHARSET);
  addch(ACS_VLINE);
  attroff(A_ALTCHARSET);
  for (size_t i = 0; i < n; ++i) {
    printw(" %s ", allBindings[i].c_str());
    attron(A_ALTCHARSET);
    addch(ACS_VLINE);
    attroff(A_ALTCHARSET);
  }
  printw(" = ");
  attron(A_ALTCHARSET);
  addch(ACS_VLINE);
  attroff(A_ALTCHARSET);
  addch('\n');

  addch('\t');
  attron(A_ALTCHARSET);
  addch(ACS_LTEE);
  for (size_t i = 0; i < n; ++i) {
    for (int j = 0; j < colWidth[i]; ++j) {
      addch(ACS_HLINE);
    }
    addch(ACS_PLUS);
  }
  for (int j = 0; j < 3; ++j) {
    addch(ACS_HLINE);
  }
  addch(ACS_RTEE);
  attroff(A_ALTCHARSET);
  addch('\n');

  // Compute truth table rows.
  std::vector<bool> value(n);

  std::function<void(int)> Go = [&](int k) {
    if (k == n) {
      Context::Substitution sub;
      for (size_t i = 0; i < n; ++i) {
        sub[allBindings[i]] = std::make_unique<ConstExpr>(value[i]);
      }
      ctx.PushSub(sub);
      auto res = expr_->Eval(ctx);
      ctx.PopSub();
      assert(ResultOK(res));

      const auto &val = std::get<std::unique_ptr<Expr>>(res);
      assert(val->GetTag() == Expr::Tag::kConst);

      addch('\t');
      attron(A_ALTCHARSET);
      addch(ACS_VLINE);
      attroff(A_ALTCHARSET);
      for (size_t i = 0; i < n; ++i) {
        addch(' ');
        addch(value[i] ? '1' : '0');
        for (int j = 0; j < colWidth[i] - 2; ++j) {
          addch(' ');
        }
        attron(A_ALTCHARSET);
        addch(ACS_VLINE);
        attroff(A_ALTCHARSET);
      }
      addch(' ');
      addch(static_cast<const ConstExpr &>(*val).GetValue() ? '1' : '0');
      addch(' ');
      attron(A_ALTCHARSET);
      addch(ACS_VLINE);
      attroff(A_ALTCHARSET);
      addch('\n');

      return;
    }
    value[k] = false;
    Go(k + 1);
    value[k] = true;
    Go(k + 1);
  };

  Go(0);

  addch('\t');
  attron(A_ALTCHARSET);
  addch(ACS_LLCORNER);
  for (size_t i = 0; i < n; ++i) {
    for (int j = 0; j < colWidth[i]; ++j) {
      addch(ACS_HLINE);
    }
    addch(ACS_BTEE);
  }
  for (int j = 0; j < 3; ++j) {
    addch(ACS_HLINE);
  }
  addch(ACS_LRCORNER);
  attroff(A_ALTCHARSET);
  addch('\n');
}

void CheckCmd::Eval(Context &ctx) const {
  auto res = expr_->Eval(ctx);
  if (!ResultOK(res)) {
    PrintErrors(res);
    return;
  }

  const auto &expr = std::get<std::unique_ptr<Expr>>(res);
  std::unique_ptr<Expr> neg = std::make_unique<NegExpr>(expr->Copy());

  solver::algorithm::Default sat, taut;
  auto satRoot = expr->ToSolver(sat);
  auto tautRoot = neg->ToSolver(taut);
  sat.AddClause({satRoot});
  taut.AddClause({tautRoot});

  auto [satRes, satModel] = sat.Solve();
  auto [tautRes, tautModel] = taut.Solve();

  if (tautRes == solver::Result::kUNSAT) {
    printw("\t∈ TAUT\n");
  } else if (satRes == solver::Result::kSAT) {
    std::vector<std::string> model;
    for (const auto &lit : satModel) {
      if (!sat.IsTemp(lit.V())) {
        auto id = sat.NameOf(lit.V());
        if (lit.IsPos()) {
          model.emplace_back(VariableIDExpr(id).ToString());
        } else {
          model.emplace_back(
              NegExpr(std::make_unique<VariableIDExpr>(id)).ToString());
        }
      }
    }
    printw("\t∈ SAT\n");
    printw("\tmodel: %s\n", Join(model, ", ").c_str());
  } else {
    printw("\t∈ UNSAT\n");
  }
}
