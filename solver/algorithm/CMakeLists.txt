file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_library(algorithm ${SOURCE_FILES})
target_link_libraries(algorithm PRIVATE solver util)
