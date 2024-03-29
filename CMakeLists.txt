cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(nmosworkspace VERSION 1.0.1)

message(STATUS "----pml/nmos ----")

#create the version information
execute_process(COMMAND ${CMAKE_COMMAND} -DNAMESPACE=nmos -DMAJOR=${PROJECT_VERSION_MAJOR} -DMINOR=${PROJECT_VERSION_MINOR} -DPATCH=${PROJECT_VERSION_PATCH} -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

SET(BUILD_TESTS OFF CACHE BOOL "set to ON to build tests")


#default expected locations for libraries
SET(DIR_BASE $ENV{HOME} CACHE STRING "base location for libraries")
SET(DIR_JSON ${DIR_BASE}/jsoncpp CACHE STRING "location of jsoncpp")
SET(DIR_LOG ${DIR_BASE}/log CACHE STRING "location of pml log")
SET(DIR_RESTGOOSE ${DIR_BASE}/Restgoose CACHE STRING "location of Restgoose")
SET(DIR_DNSSD ${DIR_BASE}/dnssd CACHE STRING "location of pml dnssd")


if(NOT DEFINED BUILD_LOG)
	SET(BUILD_LOG ON)
	add_subdirectory(${DIR_LOG} ${CMAKE_SOURCE_DIR}/build/log)
else()
	message(STATUS "nmos - pml::log being build elsewhere")
endif()

#Get Restgoose
if(NOT DEFINED BUILD_RESTGOOSE)
	SET(BUILD_RESTGOOSE ON)
	find_path(NMOS_TEMP_DIR NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE} NO_CACHE)
	if((NOT NMOS_TEMP_DIR) OR (NOT EXISTS ${NMOS_TEMP_DIR}))
		message(STATUS "Restgoose not found - clone from GitHub")
		execute_process(COMMAND git clone https://github.com/martim01/Restgoose ${DIR_RESTGOOSE})

		find_path(NMOS_TEMP_DIR2 NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE} NO_CACHE)
		if((NOT NMOS_TEMP_DIR2) OR (NOT EXISTS ${NMOS_TEMP_DIR2}))
			message(FATAL_ERROR "Failed to clone Restgoose")
		endif()
		unset(NMOS_TEMP_DIR2)
	else()
		message(STATUS "restgoose found - update to latest version")
	    execute_process(COMMAND git -C ${DIR_RESTGOOSE} pull --no-rebase)
	endif()
	unset(NMOS_TEMP_DIR)
	add_subdirectory(${DIR_RESTGOOSE} ${CMAKE_SOURCE_DIR}/build/restgoose)
else()
	message(STATUS "nmos - pml::restgoose being built elsewhere")
endif()


#Get dnssd
if(NOT DEFINED BUILD_DNSSD)
	SET(BUILD_DNSSD ON)
	find_path(NMOS_TEMP_DIR NAMES "CMakeLists.txt" PATHS ${DIR_DNSSD} NO_CACHE)
	if((NOT NMOS_TEMP_DIR) OR (NOT EXISTS ${NMOS_TEMP_DIR}))
		message(STATUS "pml::dnssd not found - clone from GitHub")
		execute_process(COMMAND git clone https://github.com/martim01/dnssd ${DIR_DNSSD})

		find_path(NMOS_TEMP_DIR2 NAMES "CMakeLists.txt" PATHS ${DIR_DNSSD} NO_CACHE)
		if((NOT NMOS_TEMP_DIR2) OR (NOT EXISTS ${NMOS_TEMP_DIR2}))
			message(FATAL_ERROR "Failed to clone dnssd")
		endif()
		unset(NMOS_TEMP_DIR2)
	else()
		message(STATUS "pml::dnssd found - update to latest version")
		execute_process(COMMAND git -C ${DIR_DNSSD} pull --no-rebase)
	endif()
	unset(NMOS_TEMP_DIR)
	add_subdirectory(${DIR_DNSSD} ${CMAKE_SOURCE_DIR}/build/dnssd)
else()
	message(STATUS "nmos - pml::dnssd being built elsewhere")
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
