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

# Find the Boost C++ libraries.

# When trying to figure out why cmake cannot find Boost, uncomment:
set(Boost_DEBUG 1)

# if linking against a custom (recent) version of boost without removing the system version, try:
# SET(Boost_USE_MULTITHREADED "NO")

FIND_PACKAGE (Boost COMPONENTS python filesystem serialization system date_time)
IF (${Boost_MINOR_VERSION} LESS 35)
  FIND_PACKAGE (Boost COMPONENTS python filesystem serialization date_time)
ENDIF (${Boost_MINOR_VERSION} LESS 35)

IF (NOT Boost_FOUND)
  # show an error message that boost was not found
  MESSAGE(FATAL_ERROR "Boost libraries were not found, please install from http://www.boost.org/")
ELSE (NOT Boost_FOUND)
  # show the boost libraries that were found
  # workaround for extra keywords showing up in Boost_LIBRARIES
  # see http://public.kitware.com/Bug/view.php?id=12887
  LIST(REMOVE_ITEM Boost_LIBRARIES "optimized")
  LIST(REMOVE_ITEM Boost_LIBRARIES "debug")
  foreach(arg ${Boost_LIBRARIES})
    MESSAGE(STATUS "Boost library: ${arg}")
  endforeach(arg ${Boost_LIBRARIES})
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

IF (WIN32)
  set(CMAKE_INCLUDE_PATH "/GnuWin32/include")
  message(STATUS "CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}")
ENDIF (WIN32)

# header include directories
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include/irrlicht )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/source/tiny )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/source )
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${PYTHON_INCLUDE_PATH} )
INCLUDE_DIRECTORIES ( ${OpenNERO_BINARY_DIR}/external/source/Irrlicht ) # irrlicht IrrCompileConfig.h generated
INCLUDE_DIRECTORIES ( ${FLEX_INCLUDE_DIRS} )
LINK_DIRECTORIES (${OpenNERO_BINARY_DIR}/external/source/tiny)

# library linking directories
IF (Boost_LIBRARY_DIRS)
  MESSAGE(STATUS "Boost library directory is located in ${Boost_LIBRARY_DIRS}")
  LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
ENDIF (Boost_LIBRARY_DIRS)

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

IF (APPLE)
  SET(MACOSX_BUNDLE_BUNDLE_NAME OpenNERO)
  SET(MACOSX_BUNDLE_GETINFO_STRING "OpenNERO  ${YEAR}.${MONTH}.${DAY} (c) 2006-2011 The University of Texas at Austin")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "edu.utexas.cs.nnrg.OpenNERO")
  SET(MACOSX_BUNDLE_SIGNATURE "NNRG") # TODO: not handled by cmake
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "${YEAR}.${MONTH}.${DAY}")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${YEAR}.${MONTH}.${DAY}")
  SET(MACOSX_BUNDLE_COPYRIGHT "(c) 2006-2011 The University of Texas at Austin")
  SET(MACOSX_BUNDLE_NIB_FILE "MainMenu")
  CONFIGURE_FILE(${CMAKE_MODULE_PATH}/OpenNERO-Info.plist ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  ADD_EXECUTABLE(OpenNERO MACOSX_BUNDLE main.cc ${OpenNERO_sources} ${OpenNERO_headers})
  GET_TARGET_PROPERTY(OpenNERO_LOCATION OpenNERO LOCATION)
  STRING(REGEX REPLACE "\\.app/Contents/MacOS/OpenNERO" "" OpenNERO_LOCATION "${OpenNERO_LOCATION}")
  SET(OpenNERO_APP_RESOURCES "${OpenNERO_LOCATION}.app/Contents/Resources")
  GET_PROPERTY(OpenNERO_JAVA_JAR GLOBAL PROPERTY OpenNERO_JAVA_JAR)
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND mkdir -p ${OpenNERO_APP_RESOURCES}
    COMMAND rsync -auv --delete-excluded --exclude='.svn' --exclude='.git' ${OpenNERO_SOURCE_DIR}/mods/* ${OpenNERO_APP_RESOURCES}
    COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/Info.plist ${OpenNERO_LOCATION}.app/Contents
    COMMAND find ${OpenNERO_LOCATION}.app/Contents -name "\".svn\"" | xargs rm -rf
    COMMAND cp -f ${OpenNERO_JAVA_JAR} ${OpenNERO_APP_RESOURCES}/TrainingWindow.jar)
  ADD_CUSTOM_TARGET(BundleStuff ALL
    COMMAND ${CMAKE_MODULE_PATH}/bundle.py ${OpenNERO_LOCATION}.app OpenNERO)
  ADD_DEPENDENCIES(BundleStuff OpenNERO)
ELSEIF (UNIX)
  ADD_EXECUTABLE(OpenNERO main.cc ${OpenNERO_sources} ${OpenNERO_headers})
  SET(OpenNERO_APP_RESOURCES "${EXECUTABLE_OUTPUT_PATH}")
  GET_PROPERTY(OpenNERO_JAVA_JAR GLOBAL PROPERTY OpenNERO_JAVA_JAR)
  # CopyStuff will always run - it copies any updated resources from the source tree into the runtime directory
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND rsync -auv --delete-excluded --exclude='.svn' --exclude='.git' ${OpenNERO_SOURCE_DIR}/mods/* ${OpenNERO_APP_RESOURCES}
    COMMAND cp -f ${OpenNERO_JAVA_JAR} ${OpenNERO_APP_RESOURCES}/TrainingWindow.jar)
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
  GET_PROPERTY(OpenNERO_JAVA_JAR GLOBAL PROPERTY OpenNERO_JAVA_JAR)
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND xcopy /S /D /I /Y "${OpenNERO_RESOURCES_DIR}*" "${OpenNERO_APP_RESOURCES}"
    COMMAND copy "${OpenNERO_JAVA_JAR}" "${OpenNERO_APP_RESOURCES}\\TrainingWindow.jar"
    VERBATIM)
  # for each Boost DLL we found, copy it to the target directory
  # and add it to the list of things we need to install
  FOREACH (boostlib ${Boost_LIBRARIES})
    STRING(REGEX REPLACE ".lib$" ".dll" boostdll ${boostlib})
    IF (EXISTS ${boostdll} AND NOT "${boostdll}" STREQUAL "debug")
      INSTALL(PROGRAMS ${boostdll} DESTINATION ".")
	  MESSAGE(STATUS "Copying boost DLL ${boostdll}")
      FILE(TO_NATIVE_PATH ${boostdll} boostdll)
      ADD_CUSTOM_COMMAND(TARGET CopyStuff POST_BUILD COMMAND xcopy /D /I "${boostdll}" "${OpenNERO_APP_RESOURCES}")
    ENDIF (EXISTS ${boostdll} AND NOT "${boostdll}" STREQUAL "debug")
  ENDFOREACH (boostlib ${Boost_LIBRARIES})
  # we also need the python dll
  STRING(REGEX REPLACE "(.*)libs/python(..)\\.lib" "\\1python\\2.dll" PYTHON_DLL ${PYTHON_LIBRARIES})
  IF (EXISTS ${PYTHON_DLL})
    FILE(TO_NATIVE_PATH ${PYTHON_DLL} PYTHON_DLL)
	MESSAGE(STATUS ${PYTHON_DLL})
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

ADD_DEPENDENCIES( CopyStuff BuildJava )

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
	PATTERN ".git" EXCLUDE
    )
