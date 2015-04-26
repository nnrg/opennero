# Overview #

OpenNERO uses the [CMake](http://www.cmake.org/) cross-platform build system. The general steps to building OpenNERO on any platform are:
  * Install prerequisites as needed
  * [Get a copy of the source code](http://code.google.com/p/opennero/source/checkout)
  * Run cmake
  * Run make
  * [Run OpenNERO](RunningOpenNero.md)
  * If you have problems, scroll through the comments on this page - someone might have solved them for you =).

# Linux #

## Prerequisites ##

Install the following packages:

#### Ubuntu ####

use: `sudo apt-get install <package1> <package2> ...`

  * subversion (needed to get the source code)
  * cmake
  * g++
  * doxygen
  * libgl1-mesa-dev
  * libx11-dev
  * libxxf86vm-dev
  * zlib1g-dev
  * libbz2-dev
  * libpng++-dev
  * libjpeg-dev
  * libboost-dev
  * libboost-python-dev
  * libboost-filesystem-dev
  * libboost-serialization-dev
  * libboost-system-dev
  * libboost-date-time-dev
  * maven
  * openjdk-7-jdk

#### Fedora ####

If you are using Fedora, trying finding the equivalent package names to the list above.  Below is a partial list:

use: `yum install <package1> <package2> ...`

  * boost-devel
  * libX11-devel
  * libXxf86vm-devel
  * mesa-libGL-devel
  * zlib-devel
  * ...

### Other packages ###

The following packages are not needed for building OpenNERO, but they are needed for running certain mods in OpenNERO.  You would need to install them whether you are building OpenNERO or downloading a precompiled binary.

  * python-tk
  * python-wxgtk2.8
  * python-matplotlib
  * python-scipy
  * python-numpy
  * python-imaging-tk

## Building OpenNERO ##

Once you have installed the prerequisites, you can continue to build OpenNERO using the following commands:

## Commands for building OpenNERO ##

  1. `mkdir opennero`
  1. `cd opennero`
  1. `svn checkout http://opennero.googlecode.com/svn/trunk`
  1. `mkdir build`
  1. `cd build`
  1. `cmake ../trunk`
  1. `make`

## Commands for running OpenNERO ##

  1. `cd dist`
  1. `./OpenNERO`

## Commands for updating build ##

  1. `cd trunk`
  1. `svn update`
  1. `cd ..\build`
  1. `make`

### Notes: ###

If you are having trouble building on Ubuntu 13.04, try running cmake with the following options:
```
$ cmake -DPYTHON_LIBRARY=/usr/lib/libpython2.7.so -DPYTHON_INCLUDE_DIR=/usr/include/python2.7 ...
```
This will force cmake to use python2.7 libraries instead of python3, which is necessary because boost is compiled against python2.7.