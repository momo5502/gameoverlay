file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_library(backend-d3d9 ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-d3d9 PUBLIC
  common
  backend
)
