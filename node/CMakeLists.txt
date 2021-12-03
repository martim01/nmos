cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(nmos_node LANGUAGES CXX)

option(BUILD_SHARED_LIB "Make a shared library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories("../include")
include_directories(${DIR_JSON}/dist)
include_directories(${DIR_LOG}/include)
include_directories("../external/uuid")
include_directories(${DIR_RESTGOOSE}/include)

SET(SOURCES "../src/activator.cpp"
 "../src/eventposter.cpp"
 "../src/is04server.cpp"
 "../src/is05server.cpp"
 "../src/nmosserver.cpp"
 "../src/nmosthread.cpp"
 "../src/nodeapi.cpp"
 "../src/nodeapiprivate.cpp"
 "../src/nodezcposter.cpp"
 "../src/resourceholder.cpp")



add_library(nmos_node SHARED ${SOURCES})

#add the os specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(nmos_node PUBLIC __GNU__)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#add the os specific defines
	target_compile_definitions(nmos_node PUBLIC __GNUWIN32__)
	target_compile_definitions(nmos_node PUBLIC _MSL_STDINT_H)
endif()


set_target_properties(nmos_node 
	PROPERTIES 
	CXX_STANDARD 14
	CXX_EXTENSIONS OFF
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE 1)

target_link_libraries(nmos_node nmos_base)
target_compile_options(nmos_node PRIVATE -O3)

set_target_properties(nmos_node PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
