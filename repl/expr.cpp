#include "repl/expr.h"

#include <sstream>
#include <unordered_map>
#include <variant>

#define PARSE_OR_RETURN(expr, f)                                               \
  do {                                                                         \
    auto res = f;                                                              \
    if (!ResultOK(res)) {                                                      \
      return std::get<std::vector<std::string>>(res);                          \
    }                                                                          \
    expr = std::move(std::get<std::unique_ptr<Expr>>(res));                    \
  } while (false)

#define CONSUME_OR_RETURN(want)                                                \
  do {                                                                         \
    if (la->type != want) {                                                    \
      std::stringstream out;                                                   \
      out << "unexpected token: got " << la->ToDescString() << ", want "       \
          << Token::TypeToDescString(want);                                    \
      std::vector<std::string> errors;                                         \
      errors.push_back(out.str());                                             \
      return errors;                                                           \
    }                                                                          \
    ++la;                                                                      \
  } while (false)

using Lookahead = std::vector<Token>::const_iterator;
using ParseResult =
    std::variant<std::unique_ptr<Expr>, std::vector<std::string>>;

static inline bool ResultOK(const ParseResult &res) {
  return std::holds_alternative<std::unique_ptr<Expr>>(res);
}

static const std::unordered_map<Token::Type, int> kBinOpPrec = {
    // Basic connectives
    {Token::Type::kConnAnd, 2},
    {Token::Type::kConnOr, 2},
    {Token::Type::kConnXor, 2},
    // Impl/Eq connectives
    {Token::Type::kConnImpl, 1},
    {Token::Type::kConnEq, 1},
};

static const std::unordered_map<Token::Type, BinaryConnective>
    kBinaryConnectiveMapping{
        // Basic connectives
        {Token::Type::kConnAnd, BinaryConnective::kAnd},
        {Token::Type::kConnOr, BinaryConnective::kOr},
        {Token::Type::kConnXor, BinaryConnective::kXor},
        // Impl/Eq connectives
        {Token::Type::kConnImpl, BinaryConnective::kImpl},
        {Token::Type::kConnEq, BinaryConnective::kEq},
    };

ParseResult ParseSubstitution(Lookahead &la);
ParseResult ParsePrimaryExpr(Lookahead &la);
ParseResult ParseTopExpr(Lookahead &la);
ParseResult ParseLetExpr(Lookahead &la);

ParseResult ParseSubstitution(Lookahead &la) {
  std::string id = la->value;
  std::map<std::string, std::unique_ptr<Expr>> subs;
  CONSUME_OR_RETURN(Token::Type::kFormulaID);
  CONSUME_OR_RETURN(Token::Type::kBracketLeft);
  if (la->type != Token::Type::kBracketRight) {
    std::string k = la->value;
    std::unique_ptr<Expr> v;
    CONSUME_OR_RETURN(Token::Type::kVariableID);
    CONSUME_OR_RETURN(Token::Type::kOpSub);
    PARSE_OR_RETURN(v, ParseTopExpr(la));
    subs.emplace(k, std::move(v));
    while (la->type == Token::Type::kOpComma) {
      CONSUME_OR_RETURN(Token::Type::kOpComma);
      k = la->value;
      CONSUME_OR_RETURN(Token::Type::kVariableID);
      CONSUME_OR_RETURN(Token::Type::kOpSub);
      PARSE_OR_RETURN(v, ParseTopExpr(la));
      subs.emplace(k, std::move(v));
    }
  }
  CONSUME_OR_RETURN(Token::Type::kBracketRight);
  return std::make_unique<SubExpr>(id, std::move(subs));
}

ParseResult ParsePrimaryExpr(Lookahead &la) {
  if (la->type == Token::Type::kConstTrue ||
      la->type == Token::Type::kConstFalse) {
    auto expr =
        std::make_unique<ConstExpr>(la->type == Token::Type::kConstTrue);
    CONSUME_OR_RETURN(la->type);
    return expr;
  } else if (la->type == Token::Type::kVariableID) {
    auto expr = std::make_unique<VariableIDExpr>(la->value);
    CONSUME_OR_RETURN(Token::Type::kVariableID);
    return expr;
  } else if (la->type == Token::Type::kFormulaID) {
    std::string id = la->value;
    CONSUME_OR_RETURN(Token::Type::kFormulaID);
    // substitution
    if (la->type == Token::Type::kBracketLeft) {
      --la;
      return ParseSubstitution(la);
    } else {
      return std::make_unique<FormulaIDExpr>(id);
    }
  } else if (la->type == Token::Type::kConnNot) {
    std::unique_ptr<Expr> rhs;
    CONSUME_OR_RETURN(Token::Type::kConnNot);
    PARSE_OR_RETURN(rhs, ParsePrimaryExpr(la));
    return std::make_unique<NegExpr>(std::move(rhs));
  } else if (la->type == Token::Type::kBracketLeft) {
    std::unique_ptr<Expr> expr;
    CONSUME_OR_RETURN(Token::Type::kBracketLeft);
    PARSE_OR_RETURN(expr, ParseTopExpr(la));
    CONSUME_OR_RETURN(Token::Type::kBracketRight);
    return expr;
  }

  std::vector<std::string> errors;
  errors.emplace_back("unexpected token: " + la->ToDescString());
  return errors;
}

ParseResult ParseBinaryExpr(Lookahead &la, std::unique_ptr<Expr> lhs,
                            int minPrec) {
  while (kBinOpPrec.count(la->type) && kBinOpPrec.at(la->type) >= minPrec) {
    const auto op = la->type;
    std::unique_ptr<Expr> rhs;

    CONSUME_OR_RETURN(la->type);
    PARSE_OR_RETURN(rhs, ParsePrimaryExpr(la));
    while (kBinOpPrec.count(la->type) &&
           kBinOpPrec.at(la->type) >= kBinOpPrec.at(op)) {
      PARSE_OR_RETURN(
          rhs, ParseBinaryExpr(la, std::move(rhs), kBinOpPrec.at(la->type)));
    }
    lhs = std::make_unique<BinExpr>(kBinaryConnectiveMapping.at(op),
                                    std::move(lhs), std::move(rhs));
  }
  return lhs;
}

ParseResult ParseTopExpr(Lookahead &la) {
  std::unique_ptr<Expr> lhs;
  PARSE_OR_RETURN(lhs, ParsePrimaryExpr(la));
  return ParseBinaryExpr(la, std::move(lhs), 0);
}

ParseResult ParseLetExpr(Lookahead &la) {
  std::unique_ptr<Expr> id;
  std::unique_ptr<Expr> expr;

  CONSUME_OR_RETURN(Token::Type::kKeywordLet);
  PARSE_OR_RETURN(id, ParsePrimaryExpr(la));
  CONSUME_OR_RETURN(Token::Type::kOpBind);
  PARSE_OR_RETURN(expr, ParseTopExpr(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);

  if (id->GetTag() != Expr::Tag::kFormulaID) {
    std::vector<std::string> errors;
    errors.push_back("'" + id->ToString() +
                     "' is not a valid formula identifier");
    return errors;
  }

  return std::make_unique<LetExpr>(
      static_cast<const FormulaIDExpr &>(*id).GetID(), std::move(expr));
}

std::unique_ptr<Expr> Expr::Parse(const std::vector<Token> &tokens,
                                  std::vector<std::string> &errors) {
  auto it = tokens.cbegin();
  if (it->type == Token::Type::kKeywordLet) {
    auto res = ParseLetExpr(it);
    if (ResultOK(res)) {
      return std::move(std::get<std::unique_ptr<Expr>>(res));
    } else {
      errors = std::get<std::vector<std::string>>(res);
      return nullptr;
    }
  } else {
    auto res = ParseTopExpr(it);
    if (ResultOK(res)) {
      return std::move(std::get<std::unique_ptr<Expr>>(res));
    } else {
      errors = std::get<std::vector<std::string>>(res);
      return nullptr;
    }
  }
}

ConstExpr::ConstExpr(bool value) : value_(value) {}

VariableIDExpr::VariableIDExpr(std::string id) : id_(id) {}

FormulaIDExpr::FormulaIDExpr(std::string id) : id_(id) {}

NegExpr::NegExpr(std::unique_ptr<Expr> rhs) : rhs_(std::move(rhs)) {}

BinExpr::BinExpr(BinaryConnective conn, std::unique_ptr<Expr> lhs,
                 std::unique_ptr<Expr> rhs)
    : conn_(conn), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

SubExpr::SubExpr(std::string id,
                 std::map<std::string, std::unique_ptr<Expr>> subs)
    : id_(id), subs_(std::move(subs)) {}

LetExpr::LetExpr(std::string id, std::unique_ptr<Expr> expr)
    : id_(id), expr_(std::move(expr)) {}
