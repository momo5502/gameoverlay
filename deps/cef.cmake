include(FetchContent)

cmake_policy(SET CMP0169 OLD)

set(CEF_ARCH "windows64")
if("${TARGET_ARCH}" STREQUAL "i386")
    set(CEF_ARCH "windows32")
endif()

set(CEF_VERSION "cef_binary_133.4.1+g02b8366+chromium-133.0.6943.142")

string(REPLACE "+" "%2B" CEF_VERSION_URLENCODED "${CEF_VERSION}")
set(CEF_URL "https://cef-builds.spotifycdn.com/${CEF_VERSION_URLENCODED}_${CEF_ARCH}.tar.bz2")
set(CEF_DIR "${CMAKE_BINARY_DIR}/${CEF_VERSION}")

message(STATUS "Fetching CEF: ${CEF_VERSION} (${CEF_ARCH})...")

FetchContent_Declare(
    cef
    URL ${CEF_URL}
    SOURCE_DIR ${CEF_DIR}
    USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_Populate(cef)
file(REMOVE_RECURSE "${cef_SOURCE_DIR}/tests")
add_subdirectory(${cef_SOURCE_DIR} ${cef_BINARY_DIR} SYSTEM)

#FetchContent_MakeAvailable(cef)

get_directory_property(CEF_STANDARD_LIBS DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_STANDARD_LIBS)
get_directory_property(CEF_BINARY_DIR DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_BINARY_DIR)
get_directory_property(CEF_SANDBOX_STANDARD_LIBS DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_SANDBOX_STANDARD_LIBS)
get_directory_property(CEF_COMPILER_DEFINES DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_COMPILER_DEFINES)
get_directory_property(CEF_COMPILER_DEFINES_DEBUG DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_COMPILER_DEFINES_DEBUG)
get_directory_property(CEF_COMPILER_DEFINES_RELEASE DIRECTORY ${cef_SOURCE_DIR} DEFINITION CEF_COMPILER_DEFINES_RELEASE)


target_include_directories(libcef_dll_wrapper INTERFACE "${cef_SOURCE_DIR}")
target_link_directories(libcef_dll_wrapper INTERFACE "${CEF_BINARY_DIR}")
target_link_libraries(libcef_dll_wrapper INTERFACE libcef cef_sandbox ${CEF_STANDARD_LIBS} ${CEF_SANDBOX_STANDARD_LIBS})

#target_compile_definitions(libcef_dll_wrapper INTERFACE ${CEF_COMPILER_DEFINES})

#foreach(def ${CEF_COMPILER_DEFINES_DEBUG})
#    target_compile_definitions(libcef_dll_wrapper INTERFACE "$<$<CONFIG:Debug>:${def}>")
#endforeach()

#foreach(def ${CEF_COMPILER_DEFINES_RELEASE})
#    target_compile_definitions(libcef_dll_wrapper INTERFACE "$<$<CONFIG:Release>:${def}>")
#endforeach()
