file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
  *.manifest
)

list(SORT SRC_FILES)

add_library(cef-loader ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_link_libraries(cef-loader PUBLIC
  libcef_dll_wrapper
)

target_link_libraries(cef-loader PRIVATE
  common
)

target_include_directories(cef-loader INTERFACE "${CMAKE_CURRENT_LIST_DIR}")

target_link_options(cef-loader PUBLIC
	"-IGNORE:4221"
	"-IGNORE:4006"
  "-IGNORE:4020"
	"/DELAYLOAD:libcef.dll"
)

set(CEF_TARGET_DIR "$<TARGET_FILE_DIR:cef-loader>/cef")

add_custom_command(TARGET cef-loader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CEF_BINARY_DIRECTORY}" "${CEF_TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CEF_BINARY_DIRECTORY}/../Resources" "${CEF_TARGET_DIR}"
)
