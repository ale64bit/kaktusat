#pragma once

#include <sstream>

namespace util {

class DimacsStream {
  class DimacsMessage {
  public:
    DimacsMessage(char h, std::stringstream &);
    ~DimacsMessage();

    template <typename T> DimacsMessage &operator<<(const T &t) {
      out_ << t;
      return *this;
    }

  private:
    std::stringstream &out_;
  };

public:
  DimacsMessage Comment();
  DimacsMessage Values();
  DimacsMessage Result();

private:
  std::stringstream stream_;

  DimacsMessage Emit(char h);
};

extern DimacsStream stream;

void InitLogging();

} // namespace util

#define LOG util::stream.Comment()
#define LOG_VALUES util::stream.Values()
#define LOG_RESULT util::stream.Result()
