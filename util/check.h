#pragma once

#include <cstdlib>
#include <iostream>

#ifndef NDEBUG
#define CHECK(msg, condition)                                                  \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << __FILE__ << ":" << __LINE__                                 \
                << ": invariant violated: '" #condition "': " << (msg)         \
                << std::endl;                                                  \
      std::abort();                                                            \
    }                                                                          \
  } while (false)
#else
#define CHECK(msg, condition)                                                  \
  do {                                                                         \
  } while (false)
#endif
