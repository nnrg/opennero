OpenNERO (c) 2007-2009 The University of Texas at Austin

http://opennero.googlecode.com/

.-------.
| ABOUT |
'-------'

OpenNERO is an open source software platform designed for research and education in 
computational intelligence in games. The project is based on the Neuro-Evolving Robotic 
Operatives (NERO) game developed by graduate and undergraduate students at the Neural 
Networks Research Group and Department of Computer Science at 
the University of Texas at Austin.

.------------.
| COMPONENTS |
'------------'

OpenNERO is built using open source components, including:

   * Irrlicht 3D Engine - released under the Irrlicht Engine License
   * Boost C++ libraries - governed by the Boost Software License
   * Python scripting language - governed by the Python License
   * rtNEAT algorithm - created by Ken Stanley and Risto Miikkulainen at UT Austin.

.-----------------------------------------------------------------------------.
| BUILDING OPENNERO (http://code.google.com/p/opennero/wiki/BuildingOpenNero) |
'-----------------------------------------------------------------------------'

OpenNERO uses the CMake cross-platform build system. The general steps are:

   * Install prerequisites as needed.
   * Check out the source code.
   * Run CMake.
   * Build.
   * Continue to Running OpenNERO

Mac OS X

Prerequisites

   * Download and install cmake
   * Install Xcode (optional install on the Mac OS X DVD)

Steps to build

   * (recommended) create a new directory for your build
   * in your build directory, run cmake -G Xcode /path/to/opennero/checkout
   * open the resulting OpenNERO.xcodeproja Xcode project file
   * Build OpenNERO using Xcode

Windows

Prerequisites

   * cmake
   * Microsoft Visual Studio

Steps to build

   * Open CMake, specify the source directory (opennero checkout directory) 
     and build directory (another directory), and generate the files for your 
     version of Visual Studio
   * Open the OpenNERO.sln solution file and build OpenNERO using Visual Studio.
   * To run from within Visual Studio, set the working directory for debugging to $(TargetDir)

Linux

Prerequisites

Make sure the following packages are installed on Ubuntu (or equivalent):

   * libboost-dev
   * libboost-date-time-dev
   * libboost-doc
   * libboost-filesystem-dev
   * libboost-python-dev
   * libboost-serialization-dev
   * libx11-dev
   * libxxf86vm-dev
   * libgl-dev
   * libz-dev

Steps to build

   * (recommended) create a new directory for your build
   * in your build directory, run cmake /path/to/opennero/checkout
   * building using make: make

To run

     cd dist
     ./OpenNERO

.----------------------------------------------------------------------------.
| BUILDING OPENNERO (http://code.google.com/p/opennero/wiki/RunningOpenNero) |
'----------------------------------------------------------------------------'

For further help, please see online documentation at:

  http://code.google.com/p/opennero/wiki/RunningOpenNero
