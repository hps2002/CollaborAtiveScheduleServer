cmake_minimum_required(VERSION 2.8)
project(hps_sf)

# include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

set(LIB_SRC
    hps_sf/log.cpp
    hps_sf/util.cpp
    hps_sf/config.cpp
    hps_sf/thread.cpp
    hps_sf/fiber.cpp
    )

add_library(hps_sf SHARED ${LIB_SRC})
# force_redefine_file_macro_for_sources(hps_sf)

set(LIB_LIB
    hps_sf
    pthread)

add_executable(test tests/test.cpp)
add_dependencies(test hps_sf)
# force_redefine_file_macro_for_sources(test)
target_link_libraries(test ${LIB_LIB} yaml-cpp)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config hps_sf)
# force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config ${LIB_LIB} yaml-cpp)

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread hps_sf)
# force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_thread ${LIB_LIB} yaml-cpp)

add_executable(test_util tests/test_util.cpp)
add_dependencies(test_util hps_sf)
# force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_util ${LIB_LIB} yaml-cpp)

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber hps_sf)
# force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_fiber ${LIB_LIB} yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
