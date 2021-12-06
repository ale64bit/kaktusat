file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_library(encoder ${SOURCE_FILES})
target_link_libraries(encoder PRIVATE solver util)
