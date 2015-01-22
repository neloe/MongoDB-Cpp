project(mongo)
cmake_minimum_required(VERSION 2.8)
option(CodeCoverage "CodeCoverage" OFF)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wno-deprecated-register ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wno-deprecated-register -O0 -g ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)
add_subdirectory(bson)
add_subdirectory(tests)
#boooooooost
#find_package(Boost 1.55 COMPONENTS thread system regex REQUIRED)

set (lib_src
     connection/mongoclient.cpp
     connection/cursor.cpp
     )

add_library(mongocpp ${lib_src})

target_link_libraries(mongocpp zmq fastbson)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)
if (CMAKE_BUILD_TYPE MATCHES Debug)
if (CodeCoverage MATCHES On)
include(CodeCoverage)
setup_target_for_coverage(${PROJECT_NAME}_coverage drivertests coverage)
setup_target_for_coverage_cobertura(${PROJECT_NAME}_cobertura drivertests coverage --gtest_output=xml:coverage.junit.xml)
endif()
endif()
