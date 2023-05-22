cmake_minimum_required(VERSION 2.8)
project(hps_sf)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

set(LIB_SRC
    hps_sf/log.cpp
    hps_sf/util.cpp
    hps_sf/config.cpp
    )

add_library(hps_sf SHARED ${LIB_SRC})
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIMES (sylar_static PROPERTIMES OUTPUT_NAME "hps_sf")

add_executable(test tests/test.cpp)
add_dependencies(test hps_sf)
target_link_libraries(test hps_sf)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config hps_sf)
target_link_libraries(test_config hps_sf yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
