cmake_minimum_required(VERSION_ 2.8.9)
project(libnmos)
set(CMAKE_BUILD_TYPE Release)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_library(libnmos SHARED, ${SOURCES})

install(TARGETS libnmos DESTINATION /usr/lib)
