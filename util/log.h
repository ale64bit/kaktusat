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

  class FatalMessage {
  public:
    FatalMessage(std::stringstream &out, const char *file, int line,
                 const char *cond);
    ~FatalMessage();

    template <typename T> FatalMessage &operator<<(const T &t) {
      out_ << t;
      return *this;
    }

  private:
    std::stringstream &out_;
  };

public:
  FatalMessage Fatal(const char *, int, const char *);
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

#define COMMENT util::stream.Comment()
#define VALUES util::stream.Values()
#define RESULT util::stream.Result()

#ifndef NDEBUG
#define LOG util::stream.Comment()
#define CHECK(cond)                                                            \
  if (!(cond))                                                                 \
  util::stream.Fatal(__FILE__, __LINE__, #cond)
#else
#define LOG                                                                    \
  if (false)                                                                   \
  util::stream.Comment()
#define CHECK(cond)                                                            \
  if (false)                                                                   \
  util::stream.Fatal(__FILE__, __LINE__, #cond)
#endif
