file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_executable(main ${SOURCE_FILES})
target_link_libraries(main PRIVATE algorithm encoder util)
