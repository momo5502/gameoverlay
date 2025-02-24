momo_silence_deprecation_warnings()

##########################################

add_subdirectory(minhook)

##########################################

option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(glew)

##########################################

include(dxsdk.cmake)

##########################################

option(BUILD_TOOLS "" OFF)

add_subdirectory(directxtk)
add_subdirectory(fx11)

##########################################

momo_restore_deprecation_warnings()
