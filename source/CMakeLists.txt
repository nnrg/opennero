# add the source files to source groups
FILE(GLOB OpenNERO_folders *)
FOREACH(f ${OpenNERO_folders})
  IF (IS_DIRECTORY ${f})
    FILE(RELATIVE_PATH folder ${CMAKE_CURRENT_SOURCE_DIR} ${f})
    FILE(GLOB_RECURSE sources_${folder} RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${folder}/*.cpp ${f}/*.h)
    SOURCE_GROUP(${folder} FILES ${sources_${folder}})
  ENDIF (IS_DIRECTORY ${f})
ENDFOREACH(f ${OpenNERO_folders})

# find all the source files
FILE(GLOB_RECURSE OpenNERO_sources *.cpp)
FILE(GLOB_RECURSE OpenNERO_headers *.h)

# find all the test source files
FILE(GLOB_RECURSE OpenNERO_tests ../test/*.cpp)

# find the Boost C++ libraries
SET(Boost_ADDITIONAL_VERSIONS 1.47.0 1.47 1.46.1 1.46.0 1.46 1.45.0 1.45 1.44.0 1.44 1.43.0 1.43 1.42.0 1.42 1.41.0 1.41 1.40.0 1.40 1.39.0 1.39 1.38.0 1.38 1.37.0 1.37)
FIND_PACKAGE (Boost COMPONENTS python filesystem serialization system date_time)
IF (${Boost_MINOR_VERSION} LESS 35)
  FIND_PACKAGE (Boost COMPONENTS python filesystem serialization date_time)
ENDIF (${Boost_MINOR_VERSION} LESS 35)

IF (NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Boost libraries were not found")
ENDIF (NOT Boost_FOUND)

# Find the Python libraries
FIND_PACKAGE ( PythonLibs )
IF (NOT PYTHON_FOUND AND PYTHON_LIBRARIES)
  SET(PYTHON_FOUND "YES")
ELSEIF (WINDOWS AND EXISTS "C:/Python27/python.exe")
  SET(PYTHON_FOUND "YES")
  SET(PYTHON_INCLUDE_PATH "C:/Python27/include")
  SET(PYTHON_LIBRARIES "C:/Python27/libs/python27.lib")
ELSE (NOT PYTHON_FOUND AND PYTHON_LIBRARIES)
  SET(PYTHON_FOUND "NO")
ENDIF(NOT PYTHON_FOUND AND PYTHON_LIBRARIES)
IF (NOT PYTHON_FOUND)
  MESSAGE(SEND_ERROR "Python not found, please install Python 2.5 or later")
ENDIF (NOT PYTHON_FOUND)
IF (PYTHON_FOUND)
  MESSAGE(STATUS "Found Python libraries in " ${PYTHON_INCLUDE_PATH} " and " ${PYTHON_LIBRARIES})
ENDIF (PYTHON_FOUND)

# header include directories
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include/irrlicht )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/source/tiny )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/source )
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${PYTHON_INCLUDE_PATH} )
INCLUDE_DIRECTORIES ( ${OpenNERO_BINARY_DIR}/external/source/Irrlicht ) # irrlicht IrrCompileConfig.h generated
LINK_DIRECTORIES (${OpenNERO_BINARY_DIR}/external/source/tiny)

# library linking directories
IF (Boost_LIBRARY_DIRS)
  LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
ENDIF (Boost_LIBRARY_DIRS)

IF (APPLE)
  SET(MACOSX_BUNDLE_BUNDLE_NAME OpenNERO)
  SET(MACOSX_BUNDLE_GETINFO_STRING "OpenNERO 0.1.0 (c) 2007-2008 The University of Texas at Austin")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "edu.utexas.cs.nnrg.OpenNERO")
  SET(MACOSX_BUNDLE_SIGNATURE "NNRG") # TODO: not handled by cmake
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "0.1")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1")
  SET(MACOSX_BUNDLE_COPYRIGHT "(c) 2007-2008 The University of Texas at Austin")
  SET(MACOSX_BUNDLE_NIB_FILE "MainMenu")
  CONFIGURE_FILE(${CMAKE_MODULE_PATH}/OpenNERO-Info.plist ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  ADD_EXECUTABLE(OpenNERO MACOSX_BUNDLE main.cc ${OpenNERO_sources} ${OpenNERO_headers})
  GET_TARGET_PROPERTY(OpenNERO_LOCATION OpenNERO LOCATION)
  STRING(REGEX REPLACE "\\.app/Contents/MacOS/OpenNERO" "" OpenNERO_LOCATION "${OpenNERO_LOCATION}")
  SET(OpenNERO_APP_RESOURCES "${OpenNERO_LOCATION}.app/Contents/Resources")
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND mkdir -p ${OpenNERO_APP_RESOURCES}
    COMMAND rsync -auv --delete-excluded --exclude='.svn' ${OpenNERO_SOURCE_DIR}/mods/* ${OpenNERO_APP_RESOURCES}
    COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/Info.plist ${OpenNERO_LOCATION}.app/Contents
    COMMAND find ${OpenNERO_LOCATION}.app/Contents -name "\".svn\"" | xargs rm -rf)
  ADD_CUSTOM_TARGET(BundleStuff ALL
    COMMAND ${CMAKE_MODULE_PATH}/bundle.py ${OpenNERO_LOCATION}.app OpenNERO)
  ADD_DEPENDENCIES(BundleStuff OpenNERO)
ELSEIF (UNIX)
  ADD_EXECUTABLE(OpenNERO main.cc ${OpenNERO_sources} ${OpenNERO_headers})
  SET(OpenNERO_APP_RESOURCES "${EXECUTABLE_OUTPUT_PATH}")
  # CopyStuff will always run - it copies any updated resources from the source tree into the runtime directory
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND rsync -auv --delete-excluded --exclude='.svn' ${OpenNERO_SOURCE_DIR}/mods/* ${OpenNERO_APP_RESOURCES})
ELSE (APPLE)
  ADD_EXECUTABLE(OpenNERO main.cc ${OpenNERO_sources} ${OpenNERO_headers})
ENDIF(APPLE)

TARGET_LINK_LIBRARIES (OpenNERO Irrlicht)
TARGET_LINK_LIBRARIES (OpenNERO tinyxml)
TARGET_LINK_LIBRARIES (OpenNERO ${PYTHON_LIBRARIES})
TARGET_LINK_LIBRARIES (OpenNERO ${Boost_LIBRARIES})

IF (APPLE)
  FIND_LIBRARY(FOUNDATION_LIB Foundation)
  FIND_LIBRARY(COCOA_LIB Cocoa)
  TARGET_LINK_LIBRARIES(OpenNERO ${PythonLibs_LIBRARIES} ${FOUNDATION_LIB} ${COCOA_LIB})
  SET_TARGET_PROPERTIES(OpenNERO PROPERTIES COMPILE_FLAGS "-include \"${OpenNERO_SOURCE_DIR}/source/core/Common.h\"")
ELSEIF (WIN32)
  # the executable on Windows depends on a few more libraries
  TARGET_LINK_LIBRARIES(OpenNERO Ws2_32)
  # on Windows, we need to copy various DLLs used by the executable into the directory where it is
  GET_TARGET_PROPERTY(OpenNERO_APP_RESOURCES OpenNERO LOCATION)
  GET_FILENAME_COMPONENT(OpenNERO_APP_RESOURCES "${OpenNERO_APP_RESOURCES}" PATH)
  FILE(TO_NATIVE_PATH "${OpenNERO_SOURCE_DIR}/mods/" OpenNERO_RESOURCES_DIR)
  FILE(TO_NATIVE_PATH "${OpenNERO_APP_RESOURCES}" OpenNERO_APP_RESOURCES)
  # CopyStuff will always get run - it uses xcopy to update only those files that are newer
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND xcopy /S /D /I /Y "${OpenNERO_RESOURCES_DIR}*" "${OpenNERO_APP_RESOURCES}"
    VERBATIM)
  # for each Boost DLL we found, copy it to the target directory
  # and add it to the list of things we need to install
  FOREACH (boostlib ${Boost_LIBRARIES})
    STRING(REGEX REPLACE ".lib$" ".dll" boostdll ${boostlib})
    IF (EXISTS ${boostdll})
      INSTALL(PROGRAMS ${boostdll} DESTINATION ".")
      FILE(TO_NATIVE_PATH ${boostdll} boostdll)
      ADD_CUSTOM_COMMAND(TARGET CopyStuff POST_BUILD COMMAND xcopy /D /I "${boostdll}" "${OpenNERO_APP_RESOURCES}")
    ENDIF (EXISTS ${boostdll})
  ENDFOREACH (boostlib ${Boost_LIBRARIES})
  # we also need the python dll
  STRING(REGEX REPLACE "(.*)libs/python(..)\\.lib" "\\1python\\2.dll" PYTHON_DLL ${PYTHON_LIBRARIES})
  IF (EXISTS ${PYTHON_DLL})
    FILE(TO_NATIVE_PATH ${PYTHON_DLL} PYTHON_DLL)
    ADD_CUSTOM_COMMAND(TARGET CopyStuff POST_BUILD COMMAND xcopy /D /I /Y "${PYTHON_DLL}" "${OpenNERO_APP_RESOURCES}")
  ENDIF (EXISTS ${PYTHON_DLL})
ELSE(APPLE)
  FIND_LIBRARY(X11_LIBRARY X11 REQUIRED)
  FIND_LIBRARY(XXF86VM_LIBRARY Xxf86vm REQUIRED)
  FIND_LIBRARY(Z_LIBRARY z REQUIRED)
  FIND_LIBRARY(OPENGL_LIBRARY GL REQUIRED)
  TARGET_LINK_LIBRARIES(OpenNERO ${X11_LIBRARY} ${XXF86VM_LIBRARY})
  TARGET_LINK_LIBRARIES(OpenNERO ${Z_LIBRARY})
  TARGET_LINK_LIBRARIES(OpenNERO ${OPENGL_LIBRARY})
ENDIF (APPLE)

# install targets
IF (APPLE)
  INSTALL(TARGETS OpenNERO
    BUNDLE DESTINATION /Applications)
ELSE (APPLE)
  INSTALL(TARGETS OpenNERO
    RUNTIME DESTINATION ".")
ENDIF (APPLE)

INSTALL(DIRECTORY ${OpenNERO_SOURCE_DIR}/mods/
        DESTINATION "."
        USE_SOURCE_PERMISSIONS
        PATTERN ".svn" EXCLUDE
    )
