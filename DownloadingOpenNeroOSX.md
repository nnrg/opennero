**Note:** This binary is outdated.  Please use the [Linux version](DownloadingOpenNero.md).  If you manage to compile OpenNERO for OS X, please let us know so we can share it with other users.

# Mac OS X #

## Download a Binary ##

  * [OpenNERO from 2013-11-17 compiled for Mac OS X 10.6 or higher (i386)](https://code.google.com/p/opennero/downloads/detail?name=OpenNERO-2013-11-17-MacOSX.dmg)

## Install Required Software ##

Python is usually already installed on Mac OS X. To check if you have a suitable version of Python and wxPython installed, start Terminal, and type in `python`.

If you get a python `>>>` prompt, check to see if you have wxPython by typing in:
```
import wx
```
If everything is working correctly, you should not see any errors when doing this. If you do get errors, go to the [wxpython site](http://www.wxpython.org/download.php) and get the wx version that matches your python version and install it. Then start python and import wx again.

On 64-bit versions of Mac OS X, you may also need to tell Python to run in 32-bit mode by default (from Terminal):
```
defaults write com.apple.versioner.python Prefer-32-Bit -bool yes
```