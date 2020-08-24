set(BHM_TEST_SRC
    Tests.cpp
    src/LevelPack/Attack.cpp
)

set(GTEST_ROOT "" CACHE PATH "Google test root directory")
set(GTEST_BUILD "" CACHE PATH "Google test cmake build directory")

# Make sure gtest lib can be found
if(NOT EXISTS "${GTEST_BUILD}/lib/Debug/gtestd.lib")
    message(FATAL_ERROR "gtest debug library can't be found. Make sure it is built in debug mode with BUILD_SHARED_LIBS.")
endif()

add_executable(BHM_test ${BHM_TEST_SRC})

include_directories($<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/BulletHellMaker/include>)

target_compile_options(BHM_test PRIVATE /bigobj)

# Include test src
target_include_directories(BHM_test PRIVATE src)

# Include SFMl, TGUI, entt, and Python
target_include_directories(BHM_test PRIVATE ${SFML_ROOT}/include)
target_include_directories(BHM_test PRIVATE ${TGUI_ROOT}/include)
target_include_directories(BHM_test PRIVATE ${ENTT_ROOT}/src)
target_include_directories(BHM_test PRIVATE ${PYTHON_ROOT}/include)
target_include_directories(BHM_test PRIVATE ${PYTHON_ROOT}/Lib/site-packages/numpy/core/include)
target_include_directories(BHM_test PRIVATE ${GTEST_ROOT}/googletest/include)
target_include_directories(BHM_test PRIVATE ${GTEST_ROOT}/googlemock/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(BHM_test ${PYTHON_ROOT}/Lib/site-packages/numpy/core/lib/npymath.lib)
    target_link_libraries(BHM_test ${PYTHON_ROOT}/libs/python27_d.lib)

    target_link_libraries(BHM_test ${SFML_ROOT}/lib/sfml-window-d.lib)
    target_link_libraries(BHM_test ${SFML_ROOT}/lib/sfml-system-d.lib)
    target_link_libraries(BHM_test ${SFML_ROOT}/lib/sfml-main-d.lib)
    target_link_libraries(BHM_test ${SFML_ROOT}/lib/sfml-graphics-d.lib)
    target_link_libraries(BHM_test ${SFML_ROOT}/lib/sfml-audio-d.lib)
    
    target_link_libraries(BHM_test ${TGUI_BUILD_DIR}/lib/Debug/tgui-d.lib)
    
    target_link_libraries(BHM_test BHM_lib)
    
    target_link_libraries(BHM_test ${GTEST_BUILD}/lib/Debug/gtestd.lib)
    target_link_libraries(BHM_test ${GTEST_BUILD}/lib/Debug/gmockd.lib)
else()
    message(FATAL_ERROR "Tests can be built only in Debug")
endif()