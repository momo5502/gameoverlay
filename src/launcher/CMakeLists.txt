file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_executable(launcher WIN32 ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(launcher PUBLIC
  common
)
