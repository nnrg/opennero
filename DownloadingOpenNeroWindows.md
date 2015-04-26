**Note:** This binary is outdated.  Please use the [Linux version](DownloadingOpenNero.md).  If you manage to compile OpenNERO for Windows, please let us know so we can share it with other users.

# Windows #

## Download a Binary ##

  * 32-bit Binary: [OpenNERO from 2013-10-10 compiled on Windows for Win32 architecture](https://code.google.com/p/opennero/downloads/detail?name=OpenNERO-2013.10.10-win32.zip)

## Install Required Software ##

To check if you have a suitable version of Python and wxPython installed, start Command Prompt (cmd), and type in `python`. If you get a python `>>>` prompt, check to see if you have wxPython by typing in:
```
import wx
```
If everything is working correctly, you should not see any errors when doing this.

  1. Download and install [Python 2.7.2 (win32)](http://www.python.org/getit/)
  1. Download and install [wxPython](http://www.wxpython.org/download.php) (required for the NERO mod).
  1. Add the directory with python.exe (usually C:\Python27) to your PATH environment variable.
    * Open Control Panel
    * Search for "Set System Environment Variables" or open the Advanced tab of System Properties.
    * Edit the "Path" variable value to include the location of your python.exe.