file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
  *.cpp
  *.h
  )

add_library(util ${SOURCE_FILES})
target_include_directories(util INTERFACE ${PROJECT_SOURCE_DIR}) # TODO: fix
