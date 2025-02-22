file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_library(backend ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend PUBLIC
  common
)

target_include_directories(backend INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
