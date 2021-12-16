cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(nmos_base LANGUAGES CXX)

option(BUILD_SHARED_LIB "Make a shared library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories("../include")
include_directories(${DIR_JSON}/dist)
include_directories(${DIR_LOG}/include)

include_directories("../external/uuid")


SET(SOURCES "../external/uuid/guid.cpp"
 "../external/uuid/md5.cpp"
 "../src/activation.cpp"
 "../src/connection.cpp"
 "../src/constraint.cpp"
 "../src/curlregister.cpp"
 "../src/device.cpp"
 "../src/flow.cpp"
 "../src/flowaudio.cpp"
 "../src/flowaudiocoded.cpp"
 "../src/flowaudioraw.cpp"
 "../src/flowdata.cpp"
 "../src/flowdatajson.cpp"
 "../src/flowdatasdianc.cpp"
 "../src/flowmux.cpp"
 "../src/flowvideo.cpp"
 "../src/flowvideocoded.cpp"
 "../src/flowvideoraw.cpp"
 "../src/ioresource.cpp"
 "../src/nmosapiversion.cpp"
 "../src/receiverbase.cpp"
 "../src/resource.cpp"
 "../src/sdp.cpp"
 "../src/self.cpp"
 "../src/senderbase.cpp"
 "../src/senderrtp.cpp"
 "../src/source.cpp"
 "../src/sourceaudio.cpp"
 "../src/sourcegeneric.cpp"
 "../src/threadpool.cpp"
 "../src/transportparams.cpp"
 "../src/utils.cpp")


#linux specific
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	#use pkgconfig to find the required libs
	find_package(PkgConfig REQUIRED QUIET)
	pkg_search_module(curl REQUIRED libcurl IMPORTED_TARGET)
	if(TARGET PkgConfig::curl)
		message(STATUS "Found libcurl")
	endif()
endif()

#Windows specific
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	
	#add the os specific defines
	add_definitions(-D__GNUWIN32__)
	add_definitions(-D_MSL_STDINT_H)
endif()


add_library(nmos_base SHARED ${SOURCES} ${OS_SOURCES})

#add the os specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(nmos_base PUBLIC __GNU__)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#add the os specific defines
	target_compile_definitions(nmos_base PUBLIC __GNUWIN32__)
	target_compile_definitions(nmos_base PUBLIC _MSL_STDINT_H)
endif()


set_target_properties(nmos_base 
	PROPERTIES 
	CXX_STANDARD 14
	CXX_EXTENSIONS OFF
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE 1)

target_link_libraries(nmos_base PkgConfig::curl)


set_target_properties(nmos_base PROPERTIES DEBUG_POSTFIX "d")

list(APPEND flags "-fPIC" "-Wall" "-fpermissive" )

if(CMAKE_BUILD_TYPE MATCHES Release)
   list(APPEND flags "-O3")
   target_compile_definitions(nmos_base PUBLIC NDEBUG DLL_EXPORTS _MSL_STDINT_H)
else()
   list(APPEND flags "-g")
   target_compile_definitions(nmos_base PUBLIC DEBUG DLL_EXPORTS _MSL_STDINT_H)
endif()

target_compile_options(nmos_base PRIVATE ${flags})

set_target_properties(nmos_base PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        install(TARGETS nmos_base LIBRARY DESTINATION /usr/local/lib)
endif()
