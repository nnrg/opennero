# OpenNERO build system for use with cmake (http://www.cmake.org/)
# to build using make on *nix:
# > cmake .
# > make

# require at least cmake 2.6
IF(COMMAND cmake_minimum_required)
  CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
ENDIF(COMMAND cmake_minimum_required)

# the project is called OpenNERO
PROJECT(OpenNERO)

# allow cmake to search for libraries
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

# default build type
IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF( NOT CMAKE_BUILD_TYPE )

# build universal on Mac OS X TODO: need universal boost libraries and internal png/jpeg/z (not fink, since those are native)
# SET( CMAKE_OSX_ARCHITECTURES "i386;ppc" )

# if we need any extra cmake modules, stick them in the make directory
SET(CMAKE_MODULE_PATH "${OpenNERO_SOURCE_DIR}/cmake")

# set old-style global output parameters
SET(EXECUTABLE_OUTPUT_PATH ${OpenNERO_BINARY_DIR}/dist)
SET(LIBRARY_OUTPUT_PATH ${OpenNERO_BINARY_DIR}/dist)
# set new-style global output parameters
SET(CMAKE_RUNTIME_OUTPUT_PATH ${OpenNERO_BINARY_DIR}/dist)

SET(UTCS_HARDY_SETUP false)
IF (UNIX)
  EXECUTE_PROCESS(COMMAND lsb_release -cs OUTPUT_VARIABLE LSB_RELEASE_CS OUTPUT_STRIP_TRAILING_WHITESPACE)
  IF (${LSB_RELEASE_CS})
    IF (${LSB_RELEASE_CS} STREQUAL "hardy" AND EXISTS "/lusr" AND EXISTS "/p")
      SET(UTCS_HARDY_SETUP true)
    ENDIF (${LSB_RELEASE_CS} STREQUAL "hardy" AND EXISTS "/lusr" AND EXISTS "/p")
  ENDIF (${LSB_RELEASE_CS})
ENDIF (UNIX)

# if we are compiling on UTCS machines, use the UTCS-specific location
IF (UTCS_HARDY_SETUP)
  IF (EXISTS "/lusr")
    LIST(APPEND CMAKE_PREFIX_PATH  "/lusr")
    LIST(APPEND CMAKE_LIBRARY_PATH "/lusr/lib")
    LIST(APPEND CMAKE_INCLUDE_PATH "/lusr/include")
  ENDIF (EXISTS "/lusr")
  IF (EXISTS "/lusr/X11")
    LIST(APPEND CMAKE_PREFIX_PATH  "/lusr/X11")
    LIST(APPEND CMAKE_LIBRARY_PATH "/lusr/X11/lib")
    LIST(APPEND CMAKE_INCLUDE_PATH "/lusr/X11/include")
  ENDIF (EXISTS "/lusr/X11")
  IF (EXISTS "/p")
    LIST(APPEND CMAKE_PREFIX_PATH  "/p")
    LIST(APPEND CMAKE_LIBRARY_PATH "/p/lib")
    LIST(APPEND CMAKE_INCLUDE_PATH "/p/include")
  ENDIF (EXISTS "/p")
ENDIF (UTCS_HARDY_SETUP)

# if the system has doxygen, make a doxygen target
INCLUDE("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)

# we need OpenGL
FIND_PACKAGE( OpenGL REQUIRED )
INCLUDE_DIRECTORIES ( ${OPENGL_INCLUDE_DIR} )

# compile external libraries
ADD_SUBDIRECTORY (${CMAKE_CURRENT_SOURCE_DIR}/external/source)
# compile java
ADD_SUBDIRECTORY (${CMAKE_CURRENT_SOURCE_DIR}/java)
# compile source
ADD_SUBDIRECTORY (${CMAKE_CURRENT_SOURCE_DIR}/source)

# TESTING
ENABLE_TESTING()

# INSTALL

# we want to create packages for the platforms using CMake and CPack
# To do this, we use INSTALL commands in source/ and external/source/
# to figure out *what to install*, and the section below to figure out
# *how to package* it.

# We have to install system libraries ourselves, but use
# InstallRequiredSystemLibraries to figure out what they are
SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP true)
INCLUDE(InstallRequiredSystemLibraries)
# Include system runtime libraries in the installation if any are
# specified by CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    IF(WIN32)
      # DLLs go same place as programs on Windows
      INSTALL(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ".")
    ELSE(WIN32)
      # .so go same place as libraries on non-Windows
      INSTALL(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ".")
    ENDIF(WIN32)
ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)

IF (WIN32)
    EXECUTE_PROCESS(COMMAND "cmd.exe" "/c" "date" "/T" OUTPUT_VARIABLE RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE ".*(..)/(..)/(....).*" "\\3" YEAR ${RESULT})
    string(REGEX REPLACE ".*(..)/(..)/(....).*" "\\1" MONTH ${RESULT})
    string(REGEX REPLACE ".*(..)/(..)/(....).*" "\\2" DAY ${RESULT})
ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE RESULT)
    string(REGEX REPLACE "(..)/(..)/(....).*" "\\3" YEAR ${RESULT})
    string(REGEX REPLACE "(..)/(..)/(....).*" "\\2" MONTH ${RESULT})
    string(REGEX REPLACE "(..)/(..)/(....).*" "\\1" DAY ${RESULT})
ELSE (WIN32)
    MESSAGE(SEND_ERROR "date not implemented")
    SET(YEAR 0000)
    SET(MONTH 00)
    SET(DAY 00)
ENDIF (WIN32)

SET(CPACK_PACKAGE_NAME "OpenNERO")
SET(CPACK_PACKAGE_VENDOR "Neural Networks Research Group")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenNERO - A Game Platform for AI Research and Education")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
SET(CPACK_PACKAGE_VERSION "${YEAR}.${MONTH}.${DAY}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${YEAR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MONTH}")
SET(CPACK_PACKAGE_VERSION_PATCH "${DAY}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenNERO")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "OpenNERO.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "OpenNERO")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\opennero.googlecode.com")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\opennero.googlecode.com")
  SET(CPACK_NSIS_CONTACT "opennero-questions@groups.google.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  #SET(CPACK_STRIP_FILES "OpenNERO")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "OpenNERO" "OpenNERO")

# cpack include
INCLUDE(CPack)

# This concludes the installation section.
