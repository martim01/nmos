cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libnmos LANGUAGES CXX)

option(BUILD_SHARED_LIB "Make a shared library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


include_directories(include)
include_directories("external/jsoncpp")
include_directories("external/uuid")


#linux specific
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	#use pkgconfig to find the required libs
	find_package(PkgConfig REQUIRED QUIET)
	pkg_search_module(microhttpd REQUIRED libmicrohttpd IMPORTED_TARGET)
	if(TARGET PkgConfig::microhttpd)
		message(STATUS "Found microhttpd")
	endif()

	pkg_search_module(curl REQUIRED libcurl IMPORTED_TARGET)
	if(TARGET PkgConfig::curl)
		message(STATUS "Found libcurl")
	endif()

	pkg_search_module(avahi REQUIRED avahi-client IMPORTED_TARGET)
	if(TARGET PkgConfig::curl)
		message(STATUS "Found avahi-client")
	endif()

	#add the os specific code
	file(GLOB OS_SOURCES "src/gnu/*.cpp")
	
	
endif()

#Windows specific
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	
	#add the os specific code
	file(GLOB OS_SOURCES "src/window/*.cpp")
	
	#add the os specific defines
	add_definitions(-D__GNUWIN32__)
	add_definitions(-D_MSL_STDINT_H)
endif()


file(GLOB SOURCES "src/*.cpp")


add_library(nmos SHARED ${OS_SOURCES} "external/uuid/guid.cpp" "external/uuid/md5.cpp" ${SOURCES} "external/jsoncpp/jsoncpp.cpp")

#add the os specific defines
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_definitions(nmos PUBLIC __GNU__)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#add the os specific defines
	target_compile_definitions(nmos PUBLIC __GNUWIN32__)
	target_compile_definitions(nmos PUBLIC _MSL_STDINT_H)
endif()


set_target_properties(nmos 
	PROPERTIES 
	CXX_STANDARD 11
	CXX_EXTENSIONS OFF
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE 1)

target_link_libraries(nmos PkgConfig::microhttpd)
target_link_libraries(nmos PkgConfig::curl)
target_link_libraries(nmos PkgConfig::avahi)


target_compile_options(nmos PRIVATE -O3)
