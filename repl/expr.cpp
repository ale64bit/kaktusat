#include "repl/expr.h"

#include <limits>
#include <unordered_map>

#include "repl/parsing.h"

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

static Result<Expr> ParseSubstitution(Lookahead &la);
static Result<Expr> ParsePrimaryExpr(Lookahead &la);
static Result<Expr> ParseTopExpr(Lookahead &la);

static Result<Expr> ParseSubstitution(Lookahead &la) {
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

static Result<Expr> ParsePrimaryExpr(Lookahead &la) {
  if (la->type == Token::Type::kConstTrue ||
      la->type == Token::Type::kConstFalse) {
    auto expr =
        std::make_unique<ConstExpr>(la->type == Token::Type::kConstTrue);
    CONSUME_OR_RETURN(la->type);
    return expr;
  } else if (la->type == Token::Type::kVariableID) {
    auto expr = std::make_unique<VariableIDExpr>(la->value);
    CONSUME_OR_RETURN(Token::Type::kVariableID);
    // TODO: probably can return a more useful error msg here for substitutions.
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

static Result<Expr> ParseBinaryExpr(Lookahead &la, std::unique_ptr<Expr> lhs,
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

static Result<Expr> ParseTopExpr(Lookahead &la) {
  std::unique_ptr<Expr> lhs;
  PARSE_OR_RETURN(lhs, ParsePrimaryExpr(la));
  return ParseBinaryExpr(la, std::move(lhs), 0);
}

Result<Expr> Expr::Parse(Lookahead &la) { return ParseTopExpr(la); }

int ConstExpr::Size() const { return 1; }
int ConstExpr::Depth() const { return 0; }

int VariableIDExpr::Size() const { return 1; }
int VariableIDExpr::Depth() const { return 0; }

int FormulaIDExpr::Size() const { return 1; }
int FormulaIDExpr::Depth() const { return 0; }

int NegExpr::Size() const { return 3 + rhs_->Size(); }
int NegExpr::Depth() const { return 1 + rhs_->Depth(); }

int BinExpr::Size() const { return 3 + lhs_->Size() + rhs_->Size(); }
int BinExpr::Depth() const {
  return 1 + std::max(lhs_->Depth(), rhs_->Depth());
}

int SubExpr::Size() const {
  assert(false);
  return std::numeric_limits<int>::min();
}
int SubExpr::Depth() const {
  assert(false);
  return std::numeric_limits<int>::min();
}
