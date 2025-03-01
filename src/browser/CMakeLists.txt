file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_library(browser ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(browser PUBLIC
  cef-loader
  common
)

target_include_directories(browser INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
