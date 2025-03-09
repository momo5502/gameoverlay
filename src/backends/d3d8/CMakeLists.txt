file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_library(backend-d3d8 ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-d3d8 PUBLIC
  hooking
  backend
)

target_link_libraries(backend-d3d8 PRIVATE
  dxsdk
)

target_include_directories(backend-d3d8 PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)