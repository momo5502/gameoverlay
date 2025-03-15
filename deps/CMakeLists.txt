momo_silence_deprecation_warnings()

##########################################

add_subdirectory(minhook)
add_subdirectory(vulkan-headers)
add_subdirectory(vulkan-loader)

##########################################

option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(glew)

##########################################

include(cef.cmake)
include(dxsdk.cmake)

##########################################

momo_restore_deprecation_warnings()
