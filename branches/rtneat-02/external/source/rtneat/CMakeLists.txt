cmake_minimum_required(VERSION 2.4)

PROJECT( rtneat )

FILE(GLOB rtneat_sources *.cpp)

# paths specific to cs.utexas.edu
IF (EXISTS "/p")
  LIST(APPEND CMAKE_PREFIX_PATH  "/p")
  LIST(APPEND CMAKE_LIBRARY_PATH "/p/lib")
  LIST(APPEND CMAKE_INCLUDE_PATH "/p/include")
ENDIF (EXISTS "/p")
IF (EXISTS "/lusr")
  LIST(APPEND CMAKE_PREFIX_PATH  "/lusr")
  LIST(APPEND CMAKE_LIBRARY_PATH "/lusr/lib")
  LIST(APPEND CMAKE_INCLUDE_PATH "/lusr/include")
ENDIF (EXISTS "/lusr")

FIND_PACKAGE(Boost)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
IF (Boost_LIBRARY_DIRS)
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_LIBRARY_DIRS)

INCLUDE_DIRECTORIES(./)

ADD_EXECUTABLE( rtneat ${rtneat_sources} )
