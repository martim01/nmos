cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(nmosworkspace VERSION 1.0.1)
include(addexternal.cmake)
message(STATUS "----pml/nmos ----")

#create the version information
execute_process(COMMAND ${CMAKE_COMMAND} -DNAMESPACE=nmos -DMAJOR=${PROJECT_VERSION_MAJOR} -DMINOR=${PROJECT_VERSION_MINOR} -DPATCH=${PROJECT_VERSION_PATCH} -DOUTPUT=${CMAKE_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

SET(BUILD_TESTS OFF CACHE BOOL "set to ON to build tests")


#default expected locations for libraries
SET(DIR_BASE ${CMAKE_SOURCE_DIR} CACHE STRING "base location for external libraries' source")
SET(DIR_JSON ${DIR_BASE}/external/jsoncpp CACHE STRING "location of jsoncpp")
SET(DIR_LOG ${DIR_BASE}/external/log CACHE STRING "location of pml log")
SET(DIR_RESTGOOSE ${DIR_BASE}/external/restgoose CACHE STRING "location of Restgoose")
SET(DIR_DNSSD ${DIR_BASE}/external/dnssd CACHE STRING "location of pml dnssd")

add_external_library(pml_log ${DIR_LOG} "martim01/log.git" "" TRUE "CMakeLists.txt")
add_external_library(restgoose ${DIR_RESTGOOSE} "martim01/restgoose.git" "origin/main" TRUE "CMakeLists.txt")
add_external_library(dnssd ${DIR_DNSSD} "martim01/dnssd.git" "origin/main" TRUE "CMakeLists.txt")
	
add_external_library(jsoncpp ${DIR_JSON} "open-source-parsers/jsoncpp.git" "" FALSE "CMakeLists.txt")
if(EXISTS ${DIR_JSON}/amalgamate.py)
	message(STATUS "Generating jsoncpp amalgamated source files")
	execute_process(COMMAND python3 amalgamate.py WORKING_DIRECTORY ${DIR_JSON})
endif()

message(STATUS "Configure nmos_base")
add_subdirectory(base)

message(STATUS "Configure nmos_node")
add_subdirectory(node)

message(STATUS "Configure nmos_client")
add_subdirectory(client)

if(${BUILD_TESTS})
	message(STATUS "Configure tests")
	add_subdirectory("tests/nmos_node_test")
	add_subdirectory("tests/nmos_client_test")
endif()
