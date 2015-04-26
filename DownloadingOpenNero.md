# Linux #

## Download a Binary ##

  * 64-bit Binary: [OpenNERO from 2/9/2015 compiled on Ubuntu 14.04.1 LTS for Intel x86\_64](http://www.cs.utexas.edu/~reza/opennero/OpenNERO-2015-02-09-x86_64.tar.gz)

## Install Required Packages ##

OpenNERO depends on the following packages:

If you do not have python installed, install it using your package manager. To do so on Ubuntu Linux, use:

```
sudo apt-get install python
```

OpenNERO depends on some boost libraries.  You may need to replace the version numbers to match what the latest binary has been linked against:

```
sudo apt-get install libboost-python1.54.0 libboost-filesystem1.54.0 libboost-serialization1.54.0 libboost-system1.54.0 libboost-date-time1.54.0
```

The current build of OpenNERO also depends on Java.  You need to install a JRE to be able to use the training window in the NERO game:

```
sudo apt-get install openjdk-7-jre
```

You also need these additional packages for running various mods:

```
sudo apt-get install python-tk python-wxgtk2.8 python-matplotlib python-scipy python-numpy python-imaging-tk 
```