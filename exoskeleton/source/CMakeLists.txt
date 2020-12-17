###########################################
## For building with the rest of OpenSim ##
###########################################

cmake_minimum_required(VERSION 3.2)

# Define project
PROJECT (exoskeletonproject)

#INCLUDE_DIRECTORIES(${OpenSim_SOURCE_DIR} ${OpenSim_SOURCE_DIR}/Vendors)

SET(OPENSIM_INSTALL_DIR $ENV{OPENSIM_HOME} 
		CACHE PATH "Top-level directory of OpenSim install")
SET(SIMBODY_INSTALL_DIR $ENV{SIMBODY_HOME} 
		CACHE PATH "Top-level directory of Simbody install")
SET(SPDLOG_INSTALL_DIR $ENV{SPDLOG_HOME} 
		CACHE PATH "Top-level directory of spdlog install")

# Change name of build target
SET(TARGET exoskeleton CACHE TYPE STRING)
# Identify the cpp file(s) that were to be built
FILE(GLOB SOURCE_FILES *.h *.cpp)
SET(SOURCE ${SOURCE_FILES})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# To add Debug feature add ";Debug" after Release on the line below
SET(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo;Release;Debug" 
      CACHE STRING "Semicolon separated list of supported configuration types, only supports Debug, Release, MinSizeRel, and RelWithDebInfo, anything else will be ignored." FORCE )

# Location of headers
#SET(SIMTK_HEADERS_DIR ${OPENSIM_INSTALL_DIR}/sdk/Simbody/include)
SET(SIMTK_HEADERS_DIR ${SIMBODY_INSTALL_DIR}/include)
SET(SPDLOG_HEADERS_DIR ${SPDLOG_INSTALL_DIR}/include)
SET(OPENSIM_HEADERS_DIR ${OPENSIM_INSTALL_DIR}/sdk/include ${OPENSIM_INSTALL_DIR}/sdk/include/OpenSim)
#SET(OPENSIM_HEADERS_DIR ${OPENSIM_INSTALL_DIR}/sdk/include/OpenSim)
INCLUDE_DIRECTORIES(${SIMTK_HEADERS_DIR} ${SPDLOG_HEADERS_DIR} ${OPENSIM_HEADERS_DIR})
# Libraries and dlls
SET(OPENSIM_LIBS_DIR ${OPENSIM_INSTALL_DIR}/sdk/lib ${OPENSIM_INSTALL_DIR}/lib)
SET(OPENSIM_DLLS_DIR ${OPENSIM_INSTALL_DIR}/bin)
SET(SIMBODY_LIBS_DIR ${SIMBODY_INSTALL_DIR}/lib)
SET(SIMBODY_DLLS_DIR ${SIMBODY_INSTALL_DIR}/bin)
SET(SPDLOG_LIBS_DIR ${SPDLOG_INSTALL_DIR}/lib/spdlog)
LINK_DIRECTORIES(${OPENSIM_LIBS_DIR} ${OPENSIM_DLLS_DIR} ${SIMBODY_LIBS_DIR} ${SIMBODY_DLLS_DIR}  ${SPDLOG_LIBS_DIR})

# Namespace
#SET(SIMBODY_DIR "" CACHE PATH "Prefix for simtk lib names, includes trailing '_'. Leave empty to use stock SimTK libraries.")
#MARK_AS_ADVANCED(SimTK_DIR)

ADD_EXECUTABLE(${TARGET} ${SOURCE})

#ADD_LIBRARY(SimTKcommon SHARED ${SimTK_DIR}/SimTKcommon.dll)
#set_target_properties(SimTKcommon PROPERTIES LINKER_LANGUAGE C)
#ADD_LIBRARY(SimTKmath SHARED ${SimTK_DIR}/SimTKmath.dll)
#set_target_properties(SimTKmath PROPERTIES LINKER_LANGUAGE C)
#ADD_LIBRARY(SimTKsimbody SHARED ${SimTK_DIR}/SimTKsimbody.dll)
#set_target_properties(SimTKsimbody PROPERTIES LINKER_LANGUAGE C)

TARGET_LINK_LIBRARIES(${TARGET}
	debug osimSimulation_d	optimized osimSimulation
	debug osimActuators_d	optimized osimActuators
	debug osimCommon_d		optimized osimCommon
	debug osimAnalyses_d	optimized osimAnalyses
	debug osimTools_d		optimized osimTools
	debug SimTKcommon_d optimized SimTKcommon
	debug SimTKmath_d optimized SimTKmath
	debug SimTKsimbody_d optimized SimTKsimbody
        debug spdlogd optimized spdlogd
	${PLATFORM_LIBS}
)

IF(WIN32)
	SET(PLATFORM_LIBS  pthreadVC2)
ELSE (WIN32)
  SET(NameSpace "")
  IF(APPLE)
	SET(PLATFORM_LIBS  SimTKAtlas)
  ELSE(APPLE)
	SET(PLTAFORM_LIBS SimTKAtlas_Lin_generic)
  ENDIF(APPLE)
ENDIF (WIN32)

# This block copies the additional files into the running directory
# For example vtp, obj files. Add to the end for more extentions
# FILE(GLOB DATA_FILES *.obj)
# FOREACH (dataFile ${DATA_FILES})
#	 ADD_CUSTOM_COMMAND(
#	    TARGET ${TARGET} POST_BUILD
#	    COMMAND ${CMAKE_COMMAND} ARGS -E copy
#		    ${dataFile}
#		    ${OpenSimTugOfWar_BINARY_DIR})
# ENDFOREACH (dataFile) 


MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)
