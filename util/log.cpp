#include "log.h"

#include <cstdlib>
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

DimacsStream::FatalMessage::FatalMessage(std::stringstream &out,
                                         const char *file, int line,
                                         const char *cond)
    : out_(out) {
  out_ << "c fatal: " << file << ":" << line << ": condition '" << cond
       << "' failed: ";
}

DimacsStream::FatalMessage::~FatalMessage() {
  std::cerr << out_.str() << std::endl;
  std::abort();
}

DimacsStream::DimacsMessage DimacsStream::Emit(char h) {
  return DimacsStream::DimacsMessage(h, stream_);
}

DimacsStream::FatalMessage DimacsStream::Fatal(const char *file, int line,
                                               const char *cond) {
  return DimacsStream::FatalMessage(stream_, file, line, cond);
}

DimacsStream::DimacsMessage DimacsStream::Comment() { return Emit('c'); }

DimacsStream::DimacsMessage DimacsStream::Values() { return Emit('v'); }

DimacsStream::DimacsMessage DimacsStream::Result() { return Emit('s'); }

void InitLogging() {
  std::ios::sync_with_stdio(false);
  std::cout.tie(nullptr);
}

} // namespace util
