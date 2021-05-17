#include "repl/token.h"

#include <cctype>

Token Token::Parse(std::string s) {
  if (s == kConstTrueRepr) {
    return Token{Token::Type::kConstTrue, ""};
  } else if (s == kConstFalseRepr) {
    return Token{Token::Type::kConstFalse, ""};
  } else if (s == kConnNotRepr) {
    return Token{Token::Type::kConnNot, ""};
  } else if (s == kConnAndRepr) {
    return Token{Token::Type::kConnAnd, ""};
  } else if (s == kConnOrRepr) {
    return Token{Token::Type::kConnOr, ""};
  } else if (s == kConnXorRepr) {
    return Token{Token::Type::kConnXor, ""};
  } else if (s == kConnImplRepr) {
    return Token{Token::Type::kConnImpl, ""};
  } else if (s == kConnEqRepr) {
    return Token{Token::Type::kConnEq, ""};
  } else if (s == kBracketLeftRepr) {
    return Token{Token::Type::kBracketLeft, ""};
  } else if (s == kBracketRightRepr) {
    return Token{Token::Type::kBracketRight, ""};
  } else if (s == kOpBindRepr) {
    return Token{Token::Type::kOpBind, ""};
  } else if (s == kOpSubRepr) {
    return Token{Token::Type::kOpSub, ""};
  } else if (s == kOpCommaRepr) {
    return Token{Token::Type::kOpComma, ""};
  } else if (s == kKeywordLetRepr) {
    return Token{Token::Type::kKeywordLet, ""};
  } else if (std::islower(s[0])) {
    return Token{Token::Type::kVariableID, s};
  } else {
    return Token{Token::Type::kFormulaID, s};
  }
}

bool Token::operator==(const Token &that) const {
  return this->type == that.type && this->value == that.value;
}

std::string Token::TypeToDescString(Token::Type type) {
  switch (type) {
  case Token::Type::kConstTrue:
  case Token::Type::kConstFalse:
    return "constant";
  case Token::Type::kConnNot:
    return "unary connective";
  case Token::Type::kConnAnd:
  case Token::Type::kConnOr:
  case Token::Type::kConnXor:
  case Token::Type::kConnImpl:
  case Token::Type::kConnEq:
    return "binary connective";
  case Token::Type::kBracketLeft:
    return "left bracket";
  case Token::Type::kBracketRight:
    return "right bracket";
  case Token::Type::kOpBind:
  case Token::Type::kOpSub:
  case Token::Type::kOpComma:
    return "operator";
  case Token::Type::kKeywordLet:
    return "keyword";
  case Token::Type::kVariableID:
    return "variable identifier";
  case Token::Type::kFormulaID:
    return "formula identifier";
  case Token::Type::kEOL:
    return "EOL";
  }
}

std::string Token::ToDescString() const {
  switch (type) {
  case Token::Type::kConstTrue:
    return TypeToDescString(type) + " " + kConstTrueRepr;
  case Token::Type::kConstFalse:
    return TypeToDescString(type) + " " + kConstFalseRepr;
  case Token::Type::kConnNot:
    return TypeToDescString(type) + " " + kConnNotRepr;
  case Token::Type::kConnAnd:
    return TypeToDescString(type) + " " + kConnAndRepr;
  case Token::Type::kConnOr:
    return TypeToDescString(type) + " " + kConnOrRepr;
  case Token::Type::kConnXor:
    return TypeToDescString(type) + " " + kConnXorRepr;
  case Token::Type::kConnImpl:
    return TypeToDescString(type) + " " + kConnImplRepr;
  case Token::Type::kConnEq:
    return TypeToDescString(type) + " " + kConnEqRepr;
  case Token::Type::kBracketLeft:
    return TypeToDescString(type) + " " + kBracketLeftRepr;
  case Token::Type::kBracketRight:
    return TypeToDescString(type) + " " + kBracketRightRepr;
  case Token::Type::kOpBind:
    return TypeToDescString(type) + " " + kOpBindRepr;
  case Token::Type::kOpSub:
    return TypeToDescString(type) + " " + kOpSubRepr;
  case Token::Type::kOpComma:
    return TypeToDescString(type) + " " + kOpCommaRepr;
  case Token::Type::kKeywordLet:
    return TypeToDescString(type) + " " + kKeywordLetRepr;
  case Token::Type::kVariableID:
    return TypeToDescString(type) + " " + value;
  case Token::Type::kFormulaID:
    return TypeToDescString(type) + " " + value;
  case Token::Type::kEOL:
    return TypeToDescString(type);
  }
}
