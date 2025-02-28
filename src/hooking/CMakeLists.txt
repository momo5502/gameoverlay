file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
)

add_library(hooking ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(hooking PUBLIC
  common
)

if (MSVC)
  target_link_libraries(hooking PUBLIC
    minhook
  )
endif()

target_include_directories(hooking INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
