**Note:** If you manage to compile OpenNERO for OS X, please let us know so we can share it with other users.

**Note:** Feel free to share your experience with building OpenNERO in the comments section.

# Mac OS X #

  * Get a copy of [OpenNERO source code](https://code.google.com/p/opennero/source/checkout)
  * Download and install [cmake](http://www.cmake.org/cmake/resources/software.html)
  * Install [Xcode](http://developer.apple.com/tools/xcode/) (optional install on the Mac OS X DVD)
  * Install [wxPython](http://www.wxpython.org/download.php)
  * Install a Java Runtime Environment (JRE)
  * Build and install [Boost Libraries](http://www.boost.org/doc/libs/release/more/getting_started/unix-variants.html)
## Steps to build ##
  1. (recommended) create a new directory for your build
  1. in your build directory, run `cmake -G Xcode /path/to/opennero/checkout`
  1. open the resulting OpenNERO.xcodeproja Xcode project file
  1. Build OpenNERO using Xcode