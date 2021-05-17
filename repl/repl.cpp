#include <algorithm>
#include <cassert>
#include <clocale>
#include <cstring>
#include <curses.h>
#include <string>
#include <vector>

#include "repl/expr.h"
#include "repl/token.h"
#include "repl/trie.h"

struct Node {
  enum class Type {
    kConnective,
    kKeyword,
    kID,
  };

  Node(std::string v, Type t) : value(v), type(t) {}

  std::string value;
  Type type;
};

constexpr const char *kPrompt = "↪  ";

// Count code points
size_t ActualSize(const std::string &s) {
  return std::count_if(s.begin(), s.end(),
                       [](char ch) { return (ch & (0xc0)) != 0x80; });
}

class InputAutomata {
public:
  InputAutomata() : state_(State::kEmpty) {
    // Keywords
    trie_.Insert("let", Node(kKeywordLetRepr, Node::Type::kKeyword));

    // Constants
    trie_.Insert("true", Node(kConstTrueRepr, Node::Type::kConnective));
    trie_.Insert("false", Node(kConstFalseRepr, Node::Type::kConnective));

    // Connectives
    trie_.Insert("not", Node(kConnNotRepr, Node::Type::kConnective));
    trie_.Insert("and", Node(kConnAndRepr, Node::Type::kConnective));
    trie_.Insert("or", Node(kConnOrRepr, Node::Type::kConnective));
    trie_.Insert("xor", Node(kConnXorRepr, Node::Type::kConnective));
    trie_.Insert("impl", Node(kConnImplRepr, Node::Type::kConnective));
    trie_.Insert("eq", Node(kConnEqRepr, Node::Type::kConnective));

    // Greek lowercase
    trie_.Insert("alpha", Node(u8"α", Node::Type::kID));
    trie_.Insert("beta", Node(u8"β", Node::Type::kID));
    trie_.Insert("gamma", Node(u8"γ", Node::Type::kID));
    trie_.Insert("delta", Node(u8"δ", Node::Type::kID));
    trie_.Insert("epsilon", Node(u8"ϵ", Node::Type::kID));
    trie_.Insert("varepsilon", Node(u8"ε", Node::Type::kID));
    trie_.Insert("zeta", Node(u8"ζ", Node::Type::kID));
    trie_.Insert("eta", Node(u8"η", Node::Type::kID));
    trie_.Insert("theta", Node(u8"θ", Node::Type::kID));
    trie_.Insert("vartheta", Node(u8"ϑ", Node::Type::kID));
    trie_.Insert("iota", Node(u8"ι", Node::Type::kID));
    trie_.Insert("kappa", Node(u8"κ", Node::Type::kID));
    trie_.Insert("lambda", Node(u8"λ", Node::Type::kID));
    trie_.Insert("mu", Node(u8"μ", Node::Type::kID));
    trie_.Insert("nu", Node(u8"ν", Node::Type::kID));
    trie_.Insert("xi", Node(u8"ξ", Node::Type::kID));
    trie_.Insert("pi", Node(u8"π", Node::Type::kID));
    trie_.Insert("rho", Node(u8"ρ", Node::Type::kID));
    trie_.Insert("varrho", Node(u8"ϱ", Node::Type::kID));
    trie_.Insert("sigma", Node(u8"σ", Node::Type::kID));
    trie_.Insert("varsigma", Node(u8"ς", Node::Type::kID));
    trie_.Insert("tau", Node(u8"τ", Node::Type::kID));
    trie_.Insert("upsilon", Node(u8"υ", Node::Type::kID));
    trie_.Insert("phi", Node(u8"φ", Node::Type::kID));
    trie_.Insert("chi", Node(u8"χ", Node::Type::kID));
    trie_.Insert("psi", Node(u8"ψ", Node::Type::kID));
    trie_.Insert("omega", Node(u8"ω", Node::Type::kID));

    // Greek uppercase
    trie_.Insert("Alpha", Node(u8"A", Node::Type::kID));
    trie_.Insert("Beta", Node(u8"B", Node::Type::kID));
    trie_.Insert("Gamma", Node(u8"Γ", Node::Type::kID));
    trie_.Insert("Delta", Node(u8"Δ", Node::Type::kID));
    trie_.Insert("Epsilon", Node(u8"E", Node::Type::kID));
    trie_.Insert("Zeta", Node(u8"Z", Node::Type::kID));
    trie_.Insert("Eta", Node(u8"Η", Node::Type::kID));
    trie_.Insert("Theta", Node(u8"Θ", Node::Type::kID));
    trie_.Insert("Iota", Node(u8"Ι", Node::Type::kID));
    trie_.Insert("Kappa", Node(u8"Κ", Node::Type::kID));
    trie_.Insert("Lambda", Node(u8"Λ", Node::Type::kID));
    trie_.Insert("Mu", Node(u8"Μ", Node::Type::kID));
    trie_.Insert("Nu", Node(u8"Ν", Node::Type::kID));
    trie_.Insert("Xi", Node(u8"Ξ", Node::Type::kID));
    trie_.Insert("Pi", Node(u8"Π", Node::Type::kID));
    trie_.Insert("Rho", Node(u8"Ρ", Node::Type::kID));
    trie_.Insert("Sigma", Node(u8"Σ", Node::Type::kID));
    trie_.Insert("Tau", Node(u8"Τ", Node::Type::kID));
    trie_.Insert("Upsilon", Node(u8"Υ", Node::Type::kID));
    trie_.Insert("Phi", Node(u8"Φ", Node::Type::kID));
    trie_.Insert("Chi", Node(u8"Χ", Node::Type::kID));
    trie_.Insert("Psi", Node(u8"Ψ", Node::Type::kID));
    trie_.Insert("Omega", Node(u8"Ω", Node::Type::kID));
  }

  void Read(char ch) {
    switch (state_) {
    case State::kEmpty:
      if (std::isblank(ch)) {
        printw("%c", ch);
        tokens_.push_back(std::string(1, ch));
      } else if (std::isalpha(ch)) {
        attron(A_ITALIC);
        printw("%c", ch);
        attroff(A_ITALIC);
        cur_ += ch;
        state_ = State::kSingle;
        cur_node_ = trie_.Next(cur_node_, ch);
      } else if (kSingleTokens.count(ch)) {
        auto tok = kSingleTokens.at(ch);
        printw("%s", tok.c_str());
        tokens_.push_back(tok);
      }
      break;
    case State::kSingle:
      if (std::isblank(ch)) {
        printw("%c", ch);
        tokens_.push_back(cur_);
        tokens_.push_back(std::string(1, ch));
        cur_ = "";
        state_ = State::kEmpty;
        cur_node_ = 0;
      } else if (std::isalpha(ch)) {
        if (trie_.HasEdge(cur_node_, ch)) {
          cur_node_ = trie_.Next(cur_node_, ch);
          if (trie_.Accepts(cur_node_)) {
            assert(trie_.IsLeaf(cur_node_));
            auto tok = trie_.Value(cur_node_).value;
            for (size_t i = 0; i < ActualSize(cur_); ++i) {
              DeleteOne();
            }
            printw("%s", tok.c_str());
            cur_ = "";
            state_ = State::kEmpty;
            cur_node_ = 0;
            tokens_.push_back(tok);
          } else {
            DeleteOne();
            cur_ += ch;
            state_ = State::kMulti;
            attron(A_UNDERLINE);
            printw("%s", cur_.c_str());
            attroff(A_UNDERLINE);
          }
        } else {
          tokens_.push_back(cur_);
          cur_ = "";
          state_ = State::kEmpty;
          cur_node_ = 0;
          Read(ch);
        }
      } else if (ch == '_') {
        printw("_");
        cur_ += "_";
        state_ = State::kSubscript;
      } else if (kSingleTokens.count(ch)) {
        auto tok = kSingleTokens.at(ch);
        printw("%s", tok.c_str());
        tokens_.push_back(cur_);
        tokens_.push_back(tok);
        cur_ = "";
        state_ = State::kEmpty;
        cur_node_ = 0;
      }
      break;
    case State::kMulti:
      if (trie_.HasEdge(cur_node_, ch)) {
        cur_node_ = trie_.Next(cur_node_, ch);
        if (trie_.Accepts(cur_node_)) {
          assert(trie_.IsLeaf(cur_node_));
          const auto tok = trie_.Value(cur_node_).value;
          for (size_t i = 0; i < ActualSize(cur_); ++i) {
            DeleteOne();
          }
          switch (trie_.Value(cur_node_).type) {
          case Node::Type::kConnective:
            printw("%s", tok.c_str());
            cur_ = "";
            state_ = State::kEmpty;
            cur_node_ = 0;
            tokens_.push_back(tok);
            break;
          case Node::Type::kKeyword:
            attron(A_BOLD);
            printw("%s", tok.c_str());
            attroff(A_BOLD);
            cur_ = "";
            state_ = State::kEmpty;
            cur_node_ = 0;
            tokens_.push_back(tok);
            break;
          case Node::Type::kID:
            attron(A_ITALIC);
            printw("%s", tok.c_str());
            attroff(A_ITALIC);
            cur_ = tok;
            // Intentionally keep the cur_node_ to disallow further completion.
            state_ = State::kSingle;
            break;
          }
        } else {
          cur_ += ch;
          attron(A_UNDERLINE);
          printw("%c", ch);
          attroff(A_UNDERLINE);
        }
      }
      break;
    case State::kSubscript:
      if (std::isblank(ch)) {
        if (cur_.back() == '_') {
          DeleteOne();
          cur_.pop_back();
        }
        printw("%c", ch);
        tokens_.push_back(cur_);
        tokens_.push_back(std::string(1, ch));
        cur_ = "";
        state_ = State::kEmpty;
        cur_node_ = 0;
      } else if (std::isdigit(ch)) {
        if (cur_.back() == '_') {
          DeleteOne();
          cur_.pop_back();
        }
        printw("%s", kSubscriptDigits[ch - '0']);
        cur_ += kSubscriptDigits[ch - '0'];
      } else if (std::islower(ch) &&
                 std::strlen(kSubscriptLetters[ch - 'a']) > 0) {
        if (cur_.back() == '_') {
          DeleteOne();
          cur_.pop_back();
        }
        printw("%s", kSubscriptLetters[ch - 'a']);
        cur_ += kSubscriptLetters[ch - 'a'];
      } else if (kSingleTokens.count(ch)) {
        if (cur_.back() == '_') {
          DeleteOne();
          cur_.pop_back();
        }
        auto tok = kSingleTokens.at(ch);
        printw("%s", tok.c_str());
        tokens_.push_back(cur_);
        tokens_.push_back(tok);
        cur_ = "";
        state_ = State::kEmpty;
        cur_node_ = 0;
      }
      break;
    }
  }

  void Erase() {
    switch (state_) {
    case State::kEmpty:
      if (!tokens_.empty()) {
        const auto tok = tokens_.back();
        tokens_.pop_back();
        for (size_t i = 0; i < ActualSize(tok); ++i) {
          DeleteOne();
        }
      }
      break;
    case State::kSingle:
      DeleteOne();
      cur_ = "";
      cur_node_ = 0;
      state_ = State::kEmpty;
      break;
    case State::kMulti:
      DeleteOne();
      cur_.pop_back();
      cur_node_ = trie_.Parent(cur_node_);
      if (cur_.size() == 1) {
        DeleteOne();
        attron(A_ITALIC);
        printw("%c", cur_[0]);
        attroff(A_ITALIC);
        state_ = State::kSingle;
      }
      break;
    case State::kSubscript:
      if (cur_.back() == '_') {
        DeleteOne();
        cur_.pop_back();
        state_ = State::kSingle;
      } else {
        // TODO what to do here?
      }
      break;
    }
  }

  void Complete(Context &ctx) {
    Read(' '); // to force flushing cur_, but could be incomplete
    if (state_ != State::kEmpty) {
      return;
    }

    std::vector<Token> tokens;
    for (const auto &tok : tokens_) {
      if (std::all_of(tok.begin(), tok.end(),
                      [](char ch) { return std::isblank(ch); })) {
        continue;
      }
      tokens.emplace_back(Token::Parse(tok));
    }
    tokens.emplace_back(Token{Token::Type::kEOL, ""});

    std::vector<std::string> errors;
    auto expr = Expr::Parse(tokens, errors);
    if (expr) {
      expr = expr->Eval(ctx);
      printw("\n\tres: %s\n", expr->ToString().c_str());
    } else {
      printw("\n\tthere are errors:\n");
      for (const auto &err : errors) {
        printw("\t\terror: %s\n", err.c_str());
      }
    }

    printw(kPrompt);
    cur_ = "";
    cur_node_ = 0;
    state_ = State::kEmpty;
    tokens_.clear();
  }

private:
  static constexpr std::array<const char *, 10> kSubscriptDigits = {
      u8"₀", u8"₁", u8"₂", u8"₃", u8"₄", u8"₅", u8"₆", u8"₇", u8"₈", u8"₉",
  };
  static constexpr std::array<const char *, 26> kSubscriptLetters = {
      u8"ₐ", u8"",  u8"",  u8"",  u8"ₑ", u8"",  u8"",  u8"ₕ", u8"ᵢ",
      u8"ⱼ", u8"ₖ", u8"ₗ", u8"ₘ", u8"ₙ", u8"ₒ", u8"ₚ", u8"",  u8"ᵣ",
      u8"ₛ", u8"ₜ", u8"ᵤ", u8"ᵥ", u8"",  u8"ₓ", u8"",  u8"",
  };

  const std::unordered_map<char, std::string> kSingleTokens = {
      // Constants
      {'1', kConstTrueRepr},
      {'0', kConstFalseRepr},
      // Connectives
      {'!', kConnNotRepr},
      {'~', kConnNotRepr},
      {'&', kConnAndRepr},
      {'|', kConnOrRepr},
      {'^', kConnXorRepr},
      // Brackets
      {'(', kBracketLeftRepr},
      {')', kBracketRightRepr},
      // Others
      {'=', kOpBindRepr},
      {'/', kOpSubRepr},
      {',', kOpCommaRepr},
  };

  void DeleteOne() const { printw("\b \b"); }

  enum class State {
    kEmpty,
    kSingle,
    kMulti,
    kSubscript,
  } state_;

  Trie<Node> trie_;
  size_t cur_node_ = 0;
  std::vector<std::string> tokens_;
  std::string cur_;
};

int main() {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  initscr();
  cbreak();
  noecho();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  Context ctx;
  InputAutomata in;

  printw(kPrompt);
  for (auto ch = getch();; ch = getch()) {
    if (ch == '\n' || ch == KEY_ENTER) {
      in.Complete(ctx);
    } else if (ch == KEY_BACKSPACE) {
      in.Erase();
    } else if (std::isprint(ch)) {
      in.Read(static_cast<char>(ch));
    } else if (ch == KEY_DOWN) {
      break;
    }
  }

  printw("\nbye!\n");
  refresh();
  getch();
  endwin();
  return 0;
}
