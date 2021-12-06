add_executable(factor factor.cpp)
target_link_libraries(factor PRIVATE algorithm encoder util)

add_executable(life life.cpp)
target_link_libraries(life PRIVATE algorithm encoder util)

add_executable(sudoku sudoku.cpp)
target_link_libraries(sudoku PRIVATE algorithm encoder util)

add_executable(taocp_7.2.2.2-44 taocp_7.2.2.2-44.cpp)
target_link_libraries(taocp_7.2.2.2-44 PRIVATE algorithm encoder util)
