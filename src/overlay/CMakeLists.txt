file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_library(overlay SHARED ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(overlay PUBLIC
  backend
  backend-d3d9
  backend-dxgi
  backend-opengl
  cef-loader
)
