file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_library(transform ${SOURCE_FILES})
target_link_libraries(transform PRIVATE solver encoder util)
