#pragma once

#include <string>

constexpr const char *kConstTrueRepr = u8"⊤";
constexpr const char *kConstFalseRepr = u8"⊥";
constexpr const char *kConnNotRepr = u8"¬";
constexpr const char *kConnAndRepr = u8"∧";
constexpr const char *kConnOrRepr = u8"∨";
constexpr const char *kConnXorRepr = u8"⊕";
constexpr const char *kConnImplRepr = u8"→";
constexpr const char *kConnEqRepr = u8"≡";
constexpr const char *kBracketLeftRepr = u8"(";
constexpr const char *kBracketRightRepr = u8")";
constexpr const char *kOpBindRepr = u8"=";
constexpr const char *kOpSubRepr = u8"/";
constexpr const char *kOpCommaRepr = u8",";
constexpr const char *kKeywordCNFRepr = u8"cnf";
constexpr const char *kKeywordCheckRepr = u8"check";
constexpr const char *kKeywordDNFRepr = u8"dnf";
constexpr const char *kKeywordDepthRepr = u8"depth";
constexpr const char *kKeywordHelpRepr = u8"help";
constexpr const char *kKeywordLetRepr = u8"let";
constexpr const char *kKeywordSizeRepr = u8"size";
constexpr const char *kKeywordTTRepr = u8"tt";

struct Token {
  enum class Type {
    kConstTrue,
    kConstFalse,
    kConnNot,
    kConnAnd,
    kConnOr,
    kConnXor,
    kConnImpl,
    kConnEq,
    kBracketLeft,
    kBracketRight,
    kOpBind,
    kOpSub,
    kOpComma,
    kKeywordCNF,
    kKeywordCheck,
    kKeywordDNF,
    kKeywordDepth,
    kKeywordHelp,
    kKeywordLet,
    kKeywordSize,
    kKeywordTT,
    kVariableID,
    kFormulaID,
    kEOL,
  } type;
  std::string value;

  bool operator==(const Token &tok) const;
  std::string ToDescString() const;

  static std::string TypeToDescString(Token::Type);
  static Token Parse(std::string s);
};
