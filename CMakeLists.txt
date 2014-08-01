project(mongo)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -g ${CMAKE_CXX_FLAGS}")

add_subdirectory(bson)
add_subdirectory(tests)
#boooooooost
find_package(Boost 1.55 COMPONENTS thread system regex REQUIRED)

set (lib_src
     connection/mongoclient.cpp
     connection/cursor.cpp
     )

set (mongo_src
     main.cpp)

add_executable(main ${mongo_src})
add_library(mongocpp ${lib_src})

target_link_libraries(main zmq fastbson mongocpp)
target_link_libraries(mongocpp zmq fastbson)
