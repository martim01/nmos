cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(nmos_client LANGUAGES CXX)

option(BUILD_SHARED_LIB "Make a shared library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories("../include")
include_directories(${DIR_JSON}/dist)
include_directories(${DIR_LOG}/include)
include_directories("../external/uuid")
include_directories(${DIR_RESTGOOSE}/include)

SET(SOURCES "../src/clientapi.cpp" 
 "../src/clientapiposter.cpp"
 "../src/clientenum.cpp"
 "../src/clientholder.cpp"
 "../src/clientposter.cpp"
 "../src/clientprivate.cpp")



add_library(nmos_client SHARED ${SOURCES})

#add the os specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(nmos_client PUBLIC __GNU__)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#add the os specific defines
	target_compile_definitions(nmos_client PUBLIC __GNUWIN32__)
	target_compile_definitions(nmos_client PUBLIC _MSL_STDINT_H)
endif()


set_target_properties(nmos_client 
	PROPERTIES 
	CXX_STANDARD 14
	CXX_EXTENSIONS OFF
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE 1)

target_link_libraries(nmos_client nmos_base)
set_target_properties(nmos_client PROPERTIES DEBUG_POSTFIX "d")

list(APPEND flags "-fPIC" "-Wall" "-fpermissive" )

if(CMAKE_BUILD_TYPE MATCHES Release)
   list(APPEND flags "-O3")
   target_compile_definitions(nmos_client PUBLIC NDEBUG DLL_EXPORTS _MSL_STDINT_H)
else()
   list(APPEND flags "-g")
   target_compile_definitions(nmos_client PUBLIC DEBUG DLL_EXPORTS _MSL_STDINT_H)
endif()

target_compile_options(nmos_client PRIVATE ${flags})



set_target_properties(nmos_client PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
