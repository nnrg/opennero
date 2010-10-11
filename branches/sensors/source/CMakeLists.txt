# find all the source files
SET(OpenNERO_sources
core/BitVector.cpp
core/Bitstream.cpp
core/Time.cpp
core/LogConnections.cpp
core/File.cpp
core/IrrSerialize.cpp
core/Log.cpp
core/LookupTable.cpp
core/IrrUtil.cpp
core/AssertExt.cpp
gui/GuiButton.cpp
gui/GuiEditBox.cpp
gui/GuiText.cpp
gui/GuiManager.cpp
gui/GuiCheckBox.cpp
gui/GuiScrollBar.cpp
gui/GuiImage.cpp
gui/GuiComboBox.cpp
gui/GuiWindow.cpp
gui/GuiContextMenu.cpp
gui/GuiBase.cpp
app.cpp
scripting/Scheduler.cpp
scripting/scripting.cpp
game/objects/TemplatedObject.cpp
game/objects/PropertyMap.cpp
game/Kernel.cpp
game/Simulation.cpp
game/SimEntityData.cpp
game/factories/SimFactory.cpp
game/factories/IrrFactory.cpp
game/SimEntity.cpp
game/SimContext.cpp
game/Mod.cpp
audio/AudioObject.cpp
audio/Ale.cpp
audio/AudioManager.cpp
input/IOMapping.cpp
input/IOState.cpp
utils/Config.cpp
ai/Environment.cpp
ai/sensors/RadarSensor.cpp
ai/sensors/Sensor.cpp
ai/sensors/RaySensor.cpp
ai/rl/tiles2.cpp
ai/rl/Sarsa.cpp
ai/rl/Approximator.cpp
ai/rl/QLearning.cpp
ai/rl/TD.cpp
ai/random/RandomAgent.cpp
ai/random/RandomAI.cpp
ai/random/RandomEnvironment.cpp
ai/AgentBrain.cpp
ai/AIObject.cpp
ai/AI.cpp
ai/PythonAI.cpp
ai/rtneat/rtNEAT.cpp
ai/AIManager.cpp
rtneat/XMLSerializable.cpp
rtneat/species.cpp
rtneat/innovation.cpp
rtneat/trait.cpp
rtneat/gene.cpp
rtneat/network.cpp
rtneat/organism.cpp
rtneat/pool.cpp
rtneat/neat.cpp
rtneat/experiments.cpp
rtneat/nnode.cpp
rtneat/link.cpp
rtneat/genome.cpp
rtneat/population.cpp
math/Random.cpp
render/Shader.cpp
render/NeroDrawNode.cpp
render/LineSet.cpp
render/FPSCounter.cpp
render/Camera.cpp
render/SceneObject.cpp
)

SET(OpenNERO_headers
core/IrrUtil.h
core/IdNameRegistry.h
core/File.h
core/Algorithm.h
core/LookupTable.h
core/IrrSerialize.h
core/Bitstream.h
core/Common.h
core/BoostCommon.h
core/ONTime.h
core/ONTypes.h
core/Error.h
core/Log.h
core/Preprocessor.h
core/TimeImpl.h
core/HashMap.h
core/LogConnections.h
core/AssertExt.h
core/BitVector.h
gui/GuiBase.h
gui/GuiContextMenu.h
gui/GuiCheckBox.h
gui/GuiText.h
gui/GuiComboBox.h
gui/GuiScrollBar.h
gui/GuiManager.h
gui/GuiWindow.h
gui/GuiEditBox.h
gui/GuiButton.h
gui/GuiImage.h
scripting/Scheduler.h
scripting/scriptIncludes.h
scripting/scripting.h
game/objects/PropertyMap.h
game/objects/TemplatedObject.h
game/objects/SimEntityComponent.h
game/factories/IrrFactory.h
game/factories/SimFactory.h
game/SimEntity.h
game/Mod.h
game/Kernel.h
game/SimEntityData.h
game/SimContext.h
game/Simulation.h
audio/AudioObject.h
audio/AudioManager.h
audio/Ale.h
input/IOState.h
input/IOEnums.h
input/IOMapping.h
utils/Config.h
mainpage.h
ai/AI.h
ai/AIObject.h
ai/sensors/RaySensor.h
ai/sensors/RadarSensor.h
ai/sensors/Sensor.h
ai/rl/TD.h
ai/rl/tiles2.h
ai/rl/Approximator.h
ai/rl/Sarsa.h
ai/rl/BackpropApproximator.h
ai/rl/QLearning.h
ai/random/RandomAI.h
ai/random/RandomEnvironment.h
ai/random/RandomAgent.h
ai/Environment.h
ai/AIManager.h
ai/rtneat/rtNEAT.h
ai/PythonAI.h
ai/AgentBrain.h
rtneat/neat.h
rtneat/innovation.h
rtneat/experiments.h
rtneat/species.h
rtneat/nnode.h
rtneat/trait.h
rtneat/genome.h
rtneat/population.h
rtneat/link.h
rtneat/network.h
rtneat/XMLSerializable.h
rtneat/pool.h
rtneat/organism.h
rtneat/mersennetwister.h
rtneat/gene.h
math/Random.h
render/FPSCounter.h
render/LineSet.h
render/Camera.h
render/NeroDrawNode.h
render/SceneObject.h
render/Shader.h
)

# find the Boost C++ libraries
SET(Boost_ADDITIONAL_VERSIONS 1.45.0 1.45 1.44.0 1.44 1.43.0 1.43 1.42.0 1.42 1.41.0 1.41 1.40.0 1.40 1.39.0 1.39 1.38.0 1.38 1.37.0 1.37)
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
ELSE (NOT PYTHON_FOUND AND PYTHON_LIBRARIES)
  SET(PYTHON_FOUND "NO")
ENDIF(NOT PYTHON_FOUND AND PYTHON_LIBRARIES)
IF (NOT PYTHON_FOUND)
  IF (WIN32)
    MESSAGE(STATUS "Python not found, using internal")
    SET( PYTHON_FOUND "YES" )
    SET( PYTHON_LIBRARIES python25.lib )
    SET( PYTHON_INCLUDE_PATH ${OpenNERO_SOURCE_DIR}/external/include/Python )
  ELSE (WIN32)
    MESSAGE(SEND_ERROR "Python not found, please install Python 2.5 or later")
  ENDIF (WIN32)
ENDIF (NOT PYTHON_FOUND)
IF (PYTHON_FOUND)
  MESSAGE(STATUS "Found Python libraries in " ${PYTHON_INCLUDE_PATH} " and " ${PYTHON_LIBRARIES})
ENDIF (PYTHON_FOUND)

# header include directories
IF (EXISTS "/p/include")
  INCLUDE_DIRECTORIES ("/p/include")
ENDIF (EXISTS "/p/include")
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include/irrlicht )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/include )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/external/source/tiny )
INCLUDE_DIRECTORIES ( ${OpenNERO_SOURCE_DIR}/source )
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${PYTHON_INCLUDE_PATH} )
INCLUDE_DIRECTORIES ( ${OpenNERO_BINARY_DIR}/external/source/Irrlicht ) # irrlicht IrrCompileConfig.h generated

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
  ADD_EXECUTABLE(OpenNERO MACOSX_BUNDLE ${OpenNERO_sources} ${OpenNERO_headers} main.cpp)
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
  ADD_EXECUTABLE(OpenNERO ${OpenNERO_sources} ${OpenNERO_headers} main.cpp)
  SET(OpenNERO_APP_RESOURCES "${EXECUTABLE_OUTPUT_PATH}")
  # CopyStuff will always run - it copies any updated resources from the source tree into the runtime directory
  ADD_CUSTOM_TARGET(CopyStuff ALL
    COMMAND rsync -auv --delete-excluded --exclude='.svn' ${OpenNERO_SOURCE_DIR}/mods/* ${OpenNERO_APP_RESOURCES})
ELSE (APPLE)
  ADD_EXECUTABLE(OpenNERO ${OpenNERO_sources} ${OpenNERO_headers} main.cpp)
ENDIF(APPLE)

ADD_DEPENDENCIES(OpenNERO Irrlicht tinyxml)

TARGET_LINK_LIBRARIES (OpenNERO Irrlicht tinyxml)
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
  SET(OpenNERO_APP_RESOURCES "${CMAKE_RUNTIME_OUTPUT_PATH}/$(Configuration)")
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

ADD_EXECUTABLE(opennero_test ${OpenNERO_sources} ${OpenNERO_headers} test.cpp)
ADD_DEPENDENCIES(opennero_test Irrlicht tinyxml)
TARGET_LINK_LIBRARIES (opennero_test Irrlicht tinyxml)
TARGET_LINK_LIBRARIES (opennero_test ${PYTHON_LIBRARIES})
TARGET_LINK_LIBRARIES (opennero_test ${Boost_LIBRARIES})
FIND_PACKAGE (Boost COMPONENTS unit_test_framework)
TARGET_LINK_LIBRARIES (opennero_test ${Boost_LIBRARIES})
