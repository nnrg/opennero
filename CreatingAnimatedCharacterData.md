## To produce an md2 file containing animated character data that can be used in OpenNERO: ##

  1. Create an animated model in a program that can export the obj format.
    * I've used Maya for this project, but obj is a simple and common format, so other programs are likely to work as well.
    * The md2 format requires that all textures for this model be contained in a single image file.
  1. Export one obj file for each frame of each animation you want your character to have
    * Example filenames: run001.obj, run002.obj, ... , stand001.obj, stand002.obj ...
    * For Maya, I have included a [script](ExportMD2Example.md) that I used to automate this process.  (This script will require some straightforward modification to match your particular animation.)
  1. Run obj\_to\_md2 on these obj files and their texture file.
    * Compile from [obj\_to\_md2.cpp](http://code.google.com/p/opennero/source/browse/trunk/misc/3d/obj_to_md2/obj_to_md2.cpp) and [Makefile](http://code.google.com/p/opennero/source/browse/trunk/misc/3d/obj_to_md2/Makefile).
    * Run obj\_to\_md2 with no arguments to display usage.