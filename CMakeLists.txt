cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(nmosworkspace)

SET(DIR_BASE $ENV{HOME} CACHE STRING "base location for libraries")
SET(DIR_JSON ${DIR_BASE}/jsoncpp CACHE STRING "location of jsoncpp")
SET(DIR_LOG ${DIR_BASE}/log CACHE STRING "location of pml log")
SET(DIR_RESTGOOSE ${DIR_BASE}/Restgoose CACHE STRING "location of Restgoose")

#Get Restgoose
find_path(TEMP_DIR NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE})
if((NOT TEMP_DIR) OR (NOT EXISTS ${TEMP_DIR}))
	message("Restgoose not found - clone from GitHub")
	execute_process(COMMAND git clone https://github.com/martim01/Restgoose ${DIR_RESTGOOSE})
endif()
unset(TEMP_DIR)

find_path(TEMP_DIR2 NAMES "CMakeLists.txt" PATHS ${DIR_RESTGOOSE})
if((NOT TEMP_DIR2) OR (NOT EXISTS ${TEMP_DIR2}))
	message(FATAL_ERROR "Failed to clone Restgoose")
endif()
unset(TEMP_DIR2)

add_subdirectory(${DIR_RESTGOOSE} ${DIR_RESTGOOSE}/build)


message(STATUS "Configure nmos_base")
add_subdirectory(base)

message(STATUS "Configure nmos_node")
add_subdirectory(node)

message(STATUS "Configure nmos_client")
add_subdirectory(client)
