cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(nmos_node LANGUAGES CXX)

option(BUILD_SHARED_LIB "Make a shared library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories("../base/include")
include_directories("../common/include")
include_directories("include")
include_directories(${DIR_JSON}/include)
include_directories(${DIR_LOG}/include)
include_directories("../external/uuid")
include_directories(${DIR_RESTGOOSE}/include)
include_directories(${DIR_DNSSD}/include)

SET(SOURCES "src/activator.cpp"
 "src/eventposter.cpp"
 "src/is04server.cpp"
 "src/is05server.cpp"
 "src/nmosserver.cpp"
 "src/nmosthread.cpp"
 "src/nodeapi.cpp"
 "src/nodeapiprivate.cpp"
 "src/nodezcposter.cpp"
 "src/resourceholder.cpp")


link_directories(${CMAKE_SOURCE_DIR}/lib)
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
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE 1)

target_link_libraries(nmos_node optimized nmos_base debug nmos_based)
target_link_libraries(nmos_node optimized pml_dnssd debug pml_dnssdd)


set_target_properties(nmos_node PROPERTIES DEBUG_POSTFIX "d")


list(APPEND flags "-fPIC" "-Wall" "-fpermissive" )

if(CMAKE_BUILD_TYPE MATCHES Release)
   list(APPEND flags "-O3")
   target_compile_definitions(nmos_node PUBLIC NDEBUG DLL_EXPORTS _MSL_STDINT_H)
else()
   list(APPEND flags "-g")
   target_compile_definitions(nmos_node PUBLIC DEBUG DLL_EXPORTS _MSL_STDINT_H)
endif()

target_compile_options(nmos_node PRIVATE ${flags})



set_target_properties(nmos_node PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        install(TARGETS nmos_node LIBRARY DESTINATION /usr/local/lib)
endif()
