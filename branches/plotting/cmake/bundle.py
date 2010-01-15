#!/usr/bin/env python

# Create the OpenNERO MacOS bundle by copying all the local dynamic libraries
# and adjusting their link paths appropriately

import sys
import os
from shutil import copy

def main():
    global bundle, bundleapp, libpath, libprefix, executable
    assert(len(sys.argv) > 2)
    bundler = Bundler(sys.argv[1], sys.argv[2])
    bundler.bundle()

class Bundler:
    LOCALS = ['/Users', '/sw', '/usr/local', '/usr/X11/lib', 'libboost'] # libraries we want to bundle

    def __init__(self, path, name):
        self.libraries = {}
        self.path = path # path of the bundle
        self.name = name # name of the bundle
        self.libpath = os.path.join(self.path,'Contents','Libraries')
        self.exepath = os.path.join(self.path,'Contents','MacOS',self.name)
        self.libprefix = '@executable_path/../Libraries/'
        if not os.path.exists(self.libpath):
            os.mkdir(self.libpath)
            print 'creating ', self.libpath
        assert(os.path.isdir(self.libpath))

    def bundle(self, target = None):
        """ make sure that everything target links to is in the bundle """
        if target == None:
            self.bundle(self.exepath)
            return
        lib_file = os.popen('otool -LX ' + target)
        assert(lib_file)
        for l in lib_file:
            id = l.strip().split()[0] # get the link id of the library
            for prefix in Bundler.LOCALS:
                if id.startswith(prefix):
                    self.addlib(id, target)
        lib_file.close()

    def addlib(self, id, target):
        libname = os.path.basename(id)
        lib = id
        newid = None
        if id in self.libraries:
            # we already copied this library, just change the target
            newid = self.libraries[id]
        else:
            if not os.path.exists(lib):
                lib = os.path.join('/usr/local/lib/',lib) # best guess
                if not os.path.exists(lib):
                    print >> sys.stderr, "Could not find library " + lib
                    sys.exit(1)
            copy(lib,self.libpath)
            newlib = os.path.join(self.libpath, libname)
            newid = self.libprefix + libname
            assert(os.system('install_name_tool -id %s %s' % (newid, newlib)) == 0)
            self.libraries[id] = newid
            print "added %s" % newid
            self.bundle(newlib)
        assert(newid)
        assert(os.system('install_name_tool -change %s %s %s' % (id, newid, target)) == 0)

if __name__ == "__main__":
    main()
