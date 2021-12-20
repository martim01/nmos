cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(nmos_node_test LANGUAGES CXX)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

SET(DIR_BASE $ENV{HOME} CACHE STRING "base location for libraries")
SET(DIR_JSON ${DIR_BASE}/jsoncpp CACHE STRING "location of jsoncpp")
SET(DIR_LOG ${DIR_BASE}/log CACHE STRING "location of pml log")

include_directories(${DIR_LOG}/include)
include_directories(${DIR_PTPMONKEY}/include)
include_directories("../../include")
include_directories(${DIR_JSON}/dist)

link_directories(${CMAKE_SOURCE_DIR}/lib)

set(SOURCES "main.cpp" "threadposter.cpp")

add_executable(nmos_node_test ${SOURCES})

#linux specific
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	#use pkgconfig to find the required libs
	find_package(PkgConfig REQUIRED)
	pkg_search_module(avahi avahi-client REQUIRED)
endif()


list(APPEND flags "-fPIC" "-Wall" "-fpermissive" "-O3")
target_compile_options(nmos_node_test PRIVATE ${flags})

list(APPEND definitions "NDEBUG" "NO_OPENSSL")

target_compile_definitions(nmos_node_test PUBLIC ${definitions})

#find external libraries
target_link_directories(nmos_node_test PUBLIC "../../lib")
target_link_libraries(nmos_node_test pml_log)
target_link_libraries(nmos_node_test pml_dnssd)
target_link_libraries(nmos_node_test avahi-client)
target_link_libraries(nmos_node_test restgoose)
target_link_libraries(nmos_node_test nmos_base)
target_link_libraries(nmos_node_test nmos_node)
target_link_libraries(nmos_node_test pthread)

set_target_properties(nmos_node_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

