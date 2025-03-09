file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_library(backend-d3d-legacy ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-d3d-legacy PUBLIC
  hooking
  backend
)

target_link_libraries(backend-d3d-legacy PRIVATE
  dxsdk
)

target_include_directories(backend-d3d-legacy PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)