cmake_minimum_required(VERSION 3.26.4)

##########################################

option(MOMO_ENABLE_SANITIZER "Enable sanitizer" OFF)

##########################################

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

##########################################

project(overlay LANGUAGES C CXX)
enable_testing()

##########################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##########################################

include(cmake/utils.cmake)
include(cmake/compiler-env.cmake)

##########################################

momo_set_new_artifact_directory()

##########################################

momo_add_subdirectory_and_get_targets("deps" EXTERNAL_TARGETS)
momo_add_subdirectory_and_get_targets("src" OWN_TARGETS)

##########################################

momo_targets_set_folder("dependencies" ${EXTERNAL_TARGETS})

momo_targets_exclude_from_all(${EXTERNAL_TARGETS})
momo_targets_disable_warnings(${EXTERNAL_TARGETS})

momo_targets_expose_includes(${OWN_TARGETS})
momo_targets_set_warnings_as_errors(${OWN_TARGETS})
