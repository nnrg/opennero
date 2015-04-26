# Prerequisites #

In order to run OpenNERO on your platform you may need to install Python and wxPython.

## On Mac OS X ##

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

## On Windows, ##

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

## On Linux, ##

If you do not have python installed, install it using your package manager. To do so on Ubuntu Linux, use:

```
sudo apt-get install python
```

You also need these additional packages for running various mods:

```
sudo apt-get install python-wxgtk2.8 python-tk python-matplotlib python-scipy python-imaging-tk 
```