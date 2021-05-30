#include "repl/cmd.h"

#include "repl/parsing.h"

LetCmd::LetCmd(std::string id, std::unique_ptr<Expr> expr)
    : id_(id), expr_(std::move(expr)) {}

NopCmd::NopCmd(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

SizeCmd::SizeCmd(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

DepthCmd::DepthCmd(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

TTCmd::TTCmd(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

static Result<Cmd> ParseLetCmd(Lookahead &la) {
  std::string id;
  std::unique_ptr<Expr> expr;

  CONSUME_OR_RETURN(Token::Type::kKeywordLet);
  id = la->value;
  CONSUME_OR_RETURN(Token::Type::kFormulaID);
  CONSUME_OR_RETURN(Token::Type::kOpBind);
  PARSE_OR_RETURN(expr, Expr::Parse(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);

  return std::make_unique<LetCmd>(id, std::move(expr));
}

static Result<Cmd> ParseNopCmd(Lookahead &la) {
  std::unique_ptr<Expr> expr;
  PARSE_OR_RETURN(expr, Expr::Parse(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);
  return std::make_unique<NopCmd>(std::move(expr));
}

static Result<Cmd> ParseSizeCmd(Lookahead &la) {
  std::unique_ptr<Expr> expr;
  CONSUME_OR_RETURN(Token::Type::kKeywordSize);
  PARSE_OR_RETURN(expr, Expr::Parse(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);
  return std::make_unique<SizeCmd>(std::move(expr));
}

static Result<Cmd> ParseDepthCmd(Lookahead &la) {
  std::unique_ptr<Expr> expr;
  CONSUME_OR_RETURN(Token::Type::kKeywordDepth);
  PARSE_OR_RETURN(expr, Expr::Parse(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);
  return std::make_unique<DepthCmd>(std::move(expr));
}

static Result<Cmd> ParseTTCmd(Lookahead &la) {
  std::unique_ptr<Expr> expr;
  CONSUME_OR_RETURN(Token::Type::kKeywordTT);
  PARSE_OR_RETURN(expr, Expr::Parse(la));
  CONSUME_OR_RETURN(Token::Type::kEOL);
  return std::make_unique<TTCmd>(std::move(expr));
}

Result<Cmd> Cmd::Parse(Lookahead &la) {
  switch (la->type) {
  case Token::Type::kKeywordLet:
    return ParseLetCmd(la);
  case Token::Type::kKeywordSize:
    return ParseSizeCmd(la);
  case Token::Type::kKeywordDepth:
    return ParseDepthCmd(la);
  case Token::Type::kKeywordTT:
    return ParseTTCmd(la);
  default:
    return ParseNopCmd(la);
  }
}
