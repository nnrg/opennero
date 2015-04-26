
---


# Introduction #

This page describes how to build OpenNERO with [Human Guided Neuroevolution](HumanGuidedNeuroevolution.md) extensions to participate in the [2012 OpenNERO tournament](NeroTournament2012.md).

# Details #

The basic process is similar to that already outlined in BuildingOpenNero, with a few changes:

  * Instead of /trunk, use [/branches/tourney2012](http://opennero.googlecode.com/svn/branches/tourney2012/) branch when checking out the code.
    * `svn checkout http://opennero.googlecode.com/svn/branches/tourney2012`
  * If your system does not have it, download and install the latest [Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads/index.html). If you want to modify the Java part of the code, we recommend also installing [Netbeans](http://netbeans.org/downloads/) or a bundle of both.
    * On Linux, use your package manager (apt-get, yum or similar) to install the netbeans package.
  * Download and install [Apache Maven](http://maven.apache.org/download.html).
  * Download and install GNU [Bison](http://www.gnu.org/software/bison/) and [Flex](http://flex.sourceforge.net/).
    * On Mac OS X, install Xcode and the command line tools.
    * On Linux, install the flex and bison packages.
    * On Windows, download the Win32 versions of [Bison](http://gnuwin32.sourceforge.net/packages/bison.htm) and [Flex](http://gnuwin32.sourceforge.net/packages/flex.htm).

Then, follow the build instructions on [Building OpenNERO](BuildingOpenNero.md) page.