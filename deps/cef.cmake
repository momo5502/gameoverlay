include(FetchContent)

SET(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RelWithDebInfo;Release;")

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

FetchContent_MakeAvailable(cef)