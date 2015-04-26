**Note:** If you manage to compile OpenNERO for Windows, please let us know so we can share it with other users.

**Note:** Feel free to share your experience with building OpenNERO in the comments section.

# Windows #

## Prerequisites ##
  * [OpenNERO source code](https://code.google.com/p/opennero/source/checkout)
  * [cmake](http://www.cmake.org/cmake/resources/software.html)
  * [http://www.microsoft.com/visualstudio/ Microsoft Visual Studio
  * [build and install Boost](http://www.boost.org/doc/libs/release/more/getting_started/windows.html)
    * Use the option `--build-type=complete` with the `.\b2 command.
    * Use the option `-jN` with `.\b2`, where N is the number of cores your computer has.
    * Do not use Boost 1.54 if you are building with Visual Studio 2013 Preview - use 1.53 instead.
  * [Latest Python 2.7.x with source](http://www.python.org/download/releases/2.7.2/)
  * [wxPython](http://www.wxpython.org/download.php)
  * Install a Java Runtime Environment (JRE)
  * Add python.exe to the Path variable (Control Panel -> System -> Environment Variables)

## Steps to build ##

  1. Open CMake, specify the source directory (opennero checkout directory) and build directory (another directory). Choose 'Suppress dev Warnings -WnoDev' from the Options menu. Click the Configure button twice, then click the Generate button.
  1. Open the OpenNERO.sln solution file and build OpenNERO using Visual Studio.
  1. To run from within Visual Studio, set OpenNERO as startup project. Also set the working directory of OpenNERO for debugging to `$(TargetDir)` (right mouse click on OpenNERO, select properties, debugging).