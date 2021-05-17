#include "repl/context.h"

Context::Context() { subStack_.push({}); }

bool Context::Exists(std::string id) const { return bindings_.count(id); }

std::unique_ptr<Expr> Context::Get(std::string id) const {
  auto it = bindings_.find(id);
  return (it != bindings_.end()) ? it->second->Copy() : nullptr;
}

std::unique_ptr<Expr> Context::SubImg(std::string id) const {
  auto it = subStack_.top().find(id);
  return (it != subStack_.top().end()) ? it->second->Copy() : nullptr;
}

void Context::Set(std::string id, std::unique_ptr<Expr> expr) {
  bindings_.erase(id);
  bindings_.emplace(id, std::move(expr));
}

void Context::PushSub(const Substitution &sub) {
  subStack_.push({});
  for (const auto &[id, expr] : sub) {
    subStack_.top().emplace(id, expr->Copy());
  }
}

void Context::PopSub() { subStack_.pop(); }
