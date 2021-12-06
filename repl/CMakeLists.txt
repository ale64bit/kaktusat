file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(repl ${SOURCE_FILES})
target_link_libraries(repl PRIVATE algorithm encoder util ${NCURSES_LIBRARIES})
