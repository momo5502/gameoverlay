file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_library(backend-vulkan ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(backend-vulkan PUBLIC
  hooking
  backend
)

#target_link_libraries(backend-vulkan PRIVATE
# libglew_static
#)

target_include_directories(backend-vulkan PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)