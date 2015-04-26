# OpenNERO Log Output #

## Introduction ##

OpenNERO produces output to the console and to a file on disk. The file is usually called **nero\_log.txt**. The location of this file depends on the system, and the kinds of messages that get printed can be controlled by editing the **logConfig.py** configuration file. This file can be used for debugging a problem or for processing the data produced during an experiment.

## Location of the log file ##

On **UNIX**, **Linux** and **Mac OS X**, the file is placed in `~/.opennero/`.

On Windows, it is placed in a user directory as well, but the location of this directory depends on the version of Windows.

On **Windows XP**, the directory is
```
%USERPROFILE%\Local Settings\Application Data\OpenNERO
```
which expands to something like
```
C:\Documents and Settings\USERNAME\Local Settings\Application Data\OpenNERO
```

On **Windows 7**, the directory is
```
%LOCALAPPDATA%\OpenNERO
```
which expands to
```
C:\Users\USERNAME\AppData\Local\OpenNERO
```

## Format of a log line ##