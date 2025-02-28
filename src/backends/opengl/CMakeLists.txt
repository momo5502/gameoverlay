file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_library(backend-opengl ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-opengl PUBLIC
  hooking
  backend
)

target_link_libraries(backend-opengl PRIVATE
 libglew_static
)

target_include_directories(backend-opengl PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)