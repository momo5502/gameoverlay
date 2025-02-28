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

if(NOT cef_POPULATED)
    FetchContent_Populate(cef)
    file(REMOVE_RECURSE "${cef_SOURCE_DIR}/tests")
    add_subdirectory(${cef_SOURCE_DIR} ${cef_BINARY_DIR})
endif()

#FetchContent_MakeAvailable(cef)
