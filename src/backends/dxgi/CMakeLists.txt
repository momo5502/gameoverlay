file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_library(backend-dxgi ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-dxgi PUBLIC
  hooking
  backend
)

target_include_directories(backend-dxgi PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)