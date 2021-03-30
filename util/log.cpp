#include "util/log.h"

#include <iostream>
#include <string>

namespace util {

DimacsStream stream;

DimacsStream::DimacsMessage::DimacsMessage(char h, std::stringstream &out)
    : out_(out) {
  // out_ << "\033[1;31m";
  out_ << h << ' ';
}

DimacsStream::DimacsMessage::~DimacsMessage() {
  // out_ << "\033[0m";
  std::cout << out_.str() << '\n';
  out_.str(std::string(""));
}

DimacsStream::DimacsMessage DimacsStream::Emit(char h) {
  return DimacsStream::DimacsMessage(h, stream_);
}
DimacsStream::DimacsMessage DimacsStream::Comment() { return Emit('c'); }
DimacsStream::DimacsMessage DimacsStream::Values() { return Emit('v'); }
DimacsStream::DimacsMessage DimacsStream::Result() { return Emit('s'); }

void InitLogging() { std::cout.tie(nullptr); }

} // namespace util
