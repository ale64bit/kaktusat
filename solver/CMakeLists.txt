file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_library(solver ${SOURCE_FILES})
target_link_libraries(solver PRIVATE util)

add_subdirectory(algorithm)
add_subdirectory(encoder)
add_subdirectory(transform)
