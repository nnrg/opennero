cmake_minimum_required(VERSION 2.4)

PROJECT( rtneat )

FILE(GLOB rtneat_sources *.cpp)

INCLUDE_DIRECTORIES(./)

ADD_EXECUTABLE( rtneat ${rtneat_sources} )
