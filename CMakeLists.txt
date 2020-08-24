cmake_minimum_required(VERSION 3.5)

# Project name and version
project(BulletHellMaker VERSION 0.1)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Macros.cmake)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set a default build type
set_option(CMAKE_BUILD_TYPE Debug STRING "Choose the type of build (Debug or Release)")

set_option(BUILD_TESTS FALSE BOOL "TRUE to build tests")
set(SFML_ROOT "" CACHE PATH "SFML root directory")
set(TGUI_ROOT "" CACHE PATH "TGUI root directory")
set(ENTT_ROOT "" CACHE PATH "entt root directory")
set(PYTHON_ROOT "" CACHE PATH "Python root directory. If you ran setup.bat, this is \"EXE_TARGET_FOLDER/packages/Python27\"")
set(TGUI_BUILD_DIR "" CACHE PATH "TGUI's cmake build directory. TGUI should be built before BulletHellMaker is built.")

# Try to find SFML's config file
if(NOT EXISTS "${SFML_ROOT}/include/SFML/Config.hpp")
    message(FATAL_ERROR "SFML headers could not be found.")
endif()

# Try to find TGUI's config file
if(NOT EXISTS "${TGUI_ROOT}/include/TGUI/Config.hpp")
    message(FATAL_ERROR "TGUI headers could not be found.")
endif()

# Try to find entt's config file
if(NOT EXISTS "${ENTT_ROOT}/src/entt/config/config.h")
    message(FATAL_ERROR "entt headers could not be found.")
endif()

# Try to find Python's libs
if(NOT (EXISTS "${PYTHON_ROOT}/libs/python27.lib" AND EXISTS "${PYTHON_ROOT}/libs/python27_d.lib"))
    message(FATAL_ERROR "Python libraries could not be found.")
endif()

# Invoke CMakeLists in src
add_subdirectory(BulletHellMaker/src)

# Build the tests if requested
if(BUILD_TESTS)
    add_subdirectory(Tests)
endif()