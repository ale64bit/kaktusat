file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_executable(batch ${SOURCE_FILES})
target_link_libraries(batch PRIVATE algorithm encoder util)
