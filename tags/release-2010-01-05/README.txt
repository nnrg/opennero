OpenNERO (c) 2007-2008 The University of Texas at Austin

NOTE: pardon our dust! this project is currently settling in to its new google.code home.

OpenNERO is an open source software platform for research and education in aritificial intelligence and games.
The project uses the following tools and libraries:
 - Irrlicht Engine (http://irrlicht.sourceforge.net/)
 - Boost C++ Libraries (http://www.boost.org/)
 - Python (http://www.python.org/)
 - Open Dynamics Engine (ODE) (http://www.ode.org/)
 - CMake build system (http://www.cmake.org/)
 - TinyXML (http://tinyxml.sourceforge.net/)

===============================================================================
| Documentation
-------------------------------------------------------------------------------

   1) http://code.google.com/p/opennero (documentation forthcoming)
   2) Run doxygen (C++ code-level documation generated in 'docs')
   3) possibly outdated AIIDE-08 demo plan in 'docs/demo'

===============================================================================
| Generating build files using CMake
-------------------------------------------------------------------------------

   1) If not already installed on your system, download and install cmake from
      http://www.cmake.org/

   2) Use cmake to generate the build or project files for your preferred 
      platform as described below.
 
===============================================================================
| Building OpenNERO on Micrsoft Windows
-------------------------------------------------------------------------------

   1) Run CMake and generate files for "Visual Studio 8 2005" or later:
       cmd> cmake -G "Visual Studio 8 2005" .

   2) Open the generated solution file OpenNERO.sln using your copy of Visual Studio

   3) Build using Visual Studio

===============================================================================
| Building OpenNERO on Linux
-------------------------------------------------------------------------------

   0) Make sure the following packages are installed on Ubuntu (or equivalent):
      * libboost-dev
      * libboost-date-time-dev
      * libboost-doc
      * libboost-filesystem-dev
      * libboost-python-dev
      * libboost-serialization-dev
      * libx11-dev
      * libxxf86vm-dev
      * libopenal-dev
      * libalut-dev
      * libgl-dev
      * libz-dev

   1) Run CMake and generate files using the default (Makefile) generator (or any other you prefer):

      > cmake .

   2) Run make to build the binaries

      > make

===============================================================================
| Building OpenNERO on Mac OS X
-------------------------------------------------------------------------------

   1) Run CMake and generate project files using the default (Makefile) generator:

      > cmake .

   2) Run make to build the binaries

      > make

**OR**

   1) Run CMake and generate project files using the XCode generator:

      > cmake -G Xcode .

   2) Open and build the resulting XCode project file

      > open OpenNERO.xcodeproj
