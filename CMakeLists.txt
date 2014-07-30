project(mongo)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "-std=c++11 -g ${CMAKE_CXX_FLAGS}")

add_subdirectory(bson)

#boooooooost
find_package(Boost 1.55 COMPONENTS thread system regex REQUIRED)

set (mongo_src
     main.cpp
     connection/mongoclient.cpp
     connection/cursor.cpp)

add_executable(main ${mongo_src} )

target_link_libraries(main zmq fastbson)
