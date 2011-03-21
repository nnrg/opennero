#!/usr/bin/env python

USAGE = """
Usage: run_experiment.py <patch> <mod> <time>

This will start OpenNERO in headless mode in order to run a 
fully automated experiment.

Here is how this works:

  - The specified patch is applied to the current directory.
        This is the complete description of the experiment, 
        as it includes any special modifications to the code, 
        constants or files on top of the OpenNERO code 
        (which could be of a specific version).

  - if this succeeds, this script starts OpenNERO in 
        headless mode with the specified starting mod as a 
        fork of this process.
        
  - if this succeeds, this script will wait for the specified 
        number of seconds, and then proceed to shut down the 
        OpenNERO process, copy its output to a file called 
        something like opennero-experiment-TIMESTAMP.log, 
        and exit with status 0.
        
  - if any of the steps mentioned above fail for any reason, 
        an error message is printed to stderr and to 
        opennero-experiment-TIMESTAMP.log, and the exit
        status is 1.
        
Parameters are:
  
  <patch> - diff file with pre-experiment changes for 
            application with patch -p0 command.
            
  <mod> - name of the mod directory to use (case sensitive)
  
  <time> - number of seconds to wait before terminating. 
           If 0, this process is replaced with the opennero 
           process and so termination will depend on that.
"""           

import os
import sys
import subprocess

def main():
    patchfile, modname, time = None, None, None
    try:
        patchfile = sys.argv[1]
        modname = sys.argv[2]
        time = int(sys.argv[3])
    except:
        print USAGE
        exit(1)

    # apply the epxeriment patch
    #patch = open(patchfile)
    #status = subprocess.call(['patch', '--dry-run', '-p2'], stdin=patch)
    #patch.close()
    status = 0

    if status != 0:
        print '*** Error: patch failed'
        exit(1)
    
    # start the mod
    status = subprocess.call(['./OpenNERO',
                              '--no-window',
                              '--mod-path=%s:common' % modname,
                              '--mod=%s' % modname,
                              '--command=import experiment'])

    if status != 0:
        print '*** Error: OpenNERO execution failed'
        exit(1)

if __name__ == "__main__":
    main()
