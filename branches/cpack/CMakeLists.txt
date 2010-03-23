# OpenNERO build system for use with cmake (http://www.cmake.org/)
# to build using make on *nix:
# > cmake .
# > make

# require at least cmake 2.4
IF(COMMAND cmake_minimum_required)
  CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
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

# if we are compiling on UTCS machines, use the UTCS-specific location
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

# if the system has doxygen, make a doxygen target
INCLUDE("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)

# we need OpenGL
FIND_PACKAGE( OpenGL REQUIRED )
INCLUDE_DIRECTORIES ( ${OPENGL_INCLUDE_DIR} )

# compile external libraries
ADD_SUBDIRECTORY (${CMAKE_CURRENT_SOURCE_DIR}/external/source)
# compile source
ADD_SUBDIRECTORY (${CMAKE_CURRENT_SOURCE_DIR}/source)

# package stuff
SET(CPACK_PACKAGE_NAME "OpenNERO")
SET(CPACK_PACKAGE_VENDOR "NNRG, UTCS, UT Austin")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenNERO - A game platform for AI research and education")
SET(CPACK_PACKAGE_VERSION "1.0.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CPack Component Example")

# cpack include
INCLUDE(CPack)
