momo_silence_deprecation_warnings()

##########################################

add_subdirectory(minhook)
add_subdirectory(fx11)

##########################################

option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(glew)

##########################################

include(dxsdk.cmake)

##########################################

momo_restore_deprecation_warnings()
