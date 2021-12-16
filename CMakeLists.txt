cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(nmosworkspace)

SET(BUILD_TESTS OFF CACHE BOOL "set to ON to build tests")
if(NOT DEFINED BUILD_LOG)
        SET(BUILD_LOG ON)
endif()

if(NOT DEFINED BUILD_DNSSD)
        SET(BUILD_DNSSD ON)
endif()

SET(DIR_BASE $ENV{HOME} CACHE STRING "base location for libraries")
SET(DIR_JSON ${DIR_BASE}/jsoncpp CACHE STRING "location of jsoncpp")
SET(DIR_LOG ${DIR_BASE}/log CACHE STRING "location of pml log")
SET(DIR_RESTGOOSE ${DIR_BASE}/Restgoose CACHE STRING "location of Restgoose")
SET(DIR_DNSSD ${DIR_BASE}/dnssd CACHE STRING "location of pml dnssd")

#Get Restgoose
find_path(NMOS_TEMP_DIR NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE} NO_CACHE)
if((NOT NMOS_TEMP_DIR) OR (NOT EXISTS ${NMOS_TEMP_DIR}))
	message(STATUS "Restgoose not found - clone from GitHub")
	execute_process(COMMAND git clone https://github.com/martim01/Restgoose ${DIR_RESTGOOSE})
endif()
unset(NMOS_TEMP_DIR)

find_path(NMOS_TEMP_DIR2 NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE} NO_CACHE)
if((NOT NMOS_TEMP_DIR2) OR (NOT EXISTS ${NMOS_TEMP_DIR2}))
	message(FATAL_ERROR "Failed to clone Restgoose")
endif()
unset(NMOS_TEMP_DIR2)


#Get dnssd
find_path(NMOS_TEMP_DIR NAMES "CMakeLists.txt" PATHS ${DIR_DNSSD} NO_CACHE)
if((NOT NMOS_TEMP_DIR) OR (NOT EXISTS ${NMOS_TEMP_DIR}))
	message(STATUS "pml::dnssd not found - clone from GitHub")
	execute_process(COMMAND git clone https://github.com/martim01/dnssd ${DIR_DNSSD})
endif()
unset(NMOS_TEMP_DIR)

find_path(NMOS_TEMP_DIR2 NAMES "CMakeLists.txt" PATHS ${DIR_DNSSD} NO_CACHE)
if((NOT NMOS_TEMP_DIR2) OR (NOT EXISTS ${NMOS_TEMP_DIR2}))
	message(FATAL_ERROR "Failed to clone dnssd")
endif()
unset(NMOS_TEMP_DIR2)

if(${BUILD_LOG})
	add_subdirectory(${DIR_LOG} ${CMAKE_SOURCE_DIR}/build/log)
else()
	message(STATUS "nmos - pml::log being build elsewhere")
endif()

SET(BUILD_LOG OFF CACHE BOOL "set to OFF if pml::log built by parent project" FORCE)
if(${BUILD_DNSSD})
	add_subdirectory(${DIR_DNSSD} ${CMAKE_SOURCE_DIR}/build/dnssd)
else()
	message(STATUS "nmos - pml::dnssd being built elsewhere")
endif()

add_subdirectory(${DIR_RESTGOOSE} ${CMAKE_SOURCE_DIR}/build/restgoose)

message(STATUS "Configure nmos_base")
add_subdirectory(base)

message(STATUS "Configure nmos_node")
add_subdirectory(node)

message(STATUS "Configure nmos_client")
add_subdirectory(client)

if(${BUILD_TESTS})
	message(STATUS "Configure tests")
	add_subdirectory("tests/nmos_node_test")
endif()
