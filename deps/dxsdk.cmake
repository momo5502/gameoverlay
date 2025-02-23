add_library(dxsdk INTERFACE)

target_include_directories(dxsdk INTERFACE "${CMAKE_CURRENT_LIST_DIR}/dxsdk/Include")

target_link_directories(dxsdk INTERFACE "${CMAKE_CURRENT_LIST_DIR}/dxsdk/Lib/$<$<STREQUAL:\"${TARGET_ARCH}\",\"x86_64\">:x64>$<$<STREQUAL:\"${TARGET_ARCH}\",\"i386\">:x86>")