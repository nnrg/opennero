/**
 * @mainpage OpenNERO: a Game Platform for AI Research and Education
 * 
 * @section intro_sec Introduction

OpenNERO (Open Neuro-Evolving Robotic Operatives) is an open source game platform designed for game AI research. The platform combines features commonly available in modern game engines (such as 3D graphics, physics simulation, 3D audio rendering, networked play, and a powerful scripting interface) with an easy-to-use API and tools for defining machine learning tasks, environments, and agents.

OpenNERO is cross-platform and can be compiled for Microsoft Windows, Mac OS X, and Linux operating systems running on x86, PPC, and x86_64 architectures. The platform can be thought of as a federation of open source libraries, including:

	- <a class="ext" href="http://irrlicht.sourceforge.net"> Irrlicht Engine</a> - a fully featured free game engine used for rendering and graphical user interface.
	- <a class="ext" href="http://www.ode.org"> Open Dynamics Engine</a> - a library used to simulate physics of rigid body collisions and dynamics.
	- <a class="ext" href="http://www.python.org"> Python scripting language</a> - OpenNERO relies on embedded Python as its scripting solution
	- <a class="ext" href="http://www.boost.org"> Boost C++ Libraries</a> - a collection of peer-reviewed libraries used for Python embedding, serialization, cross-platform file system operation, and automatic pointers
	- <a class="ext" href="http://www.openal.org"> OpenAL</a> - the audio library used for 3D audio rendering

We would like to thank the many developers of these libraries for their great contributions to the open source community.

<em>** At the time of this writing OpenNERO is under development by graduate and undergraduate students in the Neural Networks Research Group of the Department of Computer Sciences at the University of Texas at Austin. **</em>
 * 
 * @section building_sec Building
We have a new build system based on <a class="ext" href="http://www.cmake.org/"> CMake</a> version 2.6. This allows us to have one single set of build instructions that generates platform-specific and IDE-specific build files that can be then used build the project. The CMake build is stored in files called "CMakeLists.txt" that are themselves stored in the various directories of the source tree. The main one is in the root folder of OpenNERO. The procedure for building OpenNERO on your platform generally goes like this:
<ol>
<li> Download and install CMake and other dependencies (if building for the first time).

<li> Check out the project from the Subversion repository. Alternatively, you can use <a class="ext" href="https://z.cs.utexas.edu/users/nn/nero/wiki/wikka.php?wakka=HowToUseGitWithSvn"> git</a>.

<li> Run cmake to generate build instructions (such as a project for ""XCode"", Visual Studio, ""KDevelop"" or Eclipse, or a set of Makefiles).

<li> Perform the build using your selected <a class="ext" href="http://en.wikipedia.org/wiki/Integrated_development_environment"> IDE</a> (may include ""XCode"", Visual Studio, ""KDevelop"" or Eclipse) or build tool.
</ol>
Specifically, this is done as follow:

 * @subsection windows Windows
<ol>
<li> Download and install CMake and other dependencies (if building for the first time)
	<ol>
	<li> Download and install the Windows CMake package from the <a class="ext" href="http://www.cmake.org/HTML/Download.html"> CMake download site</a>

	<li> Download and install the Boost libraries from <a class="ext" href="http://www.boost-consulting.com/products/free"> Boost Consulting</a>

	<li> Download and install <a class="ext" href="http://www.microsoft.com/express/"> Microsoft Visual Studio Express Edition</a> from Microsoft. Use the ""Visual C++"" version.

	<li> Download and install a Subversion client such as <a class="ext" href="http://tortoisesvn.net/downloads"> Tortoise SVN</a>. See also: <a class="ext" href="https://z.cs.utexas.edu/users/nn/nero/wiki/wikka.php?wakka=SourceControlUsingSubversion"> Source Control Using Subversion</a>.
	</ol>
<li> Check out the project from the Subversion repository

<div class="code" style="font-family: monospace;">
		svn checkout http://opennero.googlecode.com/svn/trunk/ 
</div>
		
<li> Run cmake to generate build instructions

	- Open the CMake GUI and use it to generate a Visual Studio solution for your version of visual studio. You can also run the tool from command line.

<li> Perform the build using your selected IDE or build tool
	<ol>
	<li> Open the generated solution in Visual Studio
	<li> Select the build configuration (Release or Debug or whatever) and click Build
	<li> When debugging or running the application, be sure to set the current working directory to build/
	</ol>
</ol>

 * @subsection mac Mac OS X
<ol>
<li> Download and install CMake and other dependencies (if building for the first time)
	<ol>
	<li> Download and install the Windows CMake package from the <a class="ext" href="http://www.cmake.org/HTML/Download.html"> CMake download site</a>

	<li> Download and build the Boost libraries by following <a class="ext" href="http://www.boost.org/doc/libs/1_35_0/more/getting_started/unix-variants.html"> Boost Getting Started</a>.

	<li> Install an IDE (XCode or Eclipse) if desired
	</ol>

<li> Check out the project from the Subversion repository

<div class="code" style="font-family: monospace;">
		svn checkout http://opennero.googlecode.com/svn/trunk/
</div>

<li> Run cmake to generate build instructions

		+ Run cmake with to generate Makefile, XCode or Eclipse project:

<div class="code" style="font-family: monospace;">
$ cmake .
</div>

or

<div class="code" style="font-family: monospace;">
$ cmake -G "Xcode" .
</div>

or

<div class="code" style="font-family: monospace;">
$ cmake -G "Eclipse CDT4 - Unix Makefiles" .
</div>

<li> Perform the build using your selected IDE or build tool
</ol>

 * @subsection linux Linux
<ol>
<li> Download and install CMake and other dependencies. Open the Terminal (On upper left corner click Application -> Accessories -> Terminal), copy the following lines and hit enter. Enter your correct password (your Linux super-user password, not your UTCS password) when prompted.
	<ol>
	<li> update package info: 

<div class="code" style="font-family: monospace;">
sudo apt-get update
</div>

	<li> upgrade to latest versions of installed packages: 

<div class="code" style="font-family: monospace;">
sudo apt-get dist-upgrade
<div class="code" style="font-family: monospace;">

	<li> make sure dependencies are installed: 

<div class="code" style="font-family: monospace;">
sudo apt-get install build-essential libboost-filesystem-dev libboost-python-dev libboost-date-time-dev libboost-serialization-dev libboost-test-dev cmake
</div>

	<li> install these dependencies as well: 

<div class="code" style="font-family: monospace;">
sudo apt-get install libogg-dev libvorbis-dev libx11-dev libxxf86vm-dev libopenal-dev libalut-dev mesa-common-dev zlib1g-dev libjpeg62-dev libglpng-dev
</div>

	</ol>
<li> Check out the project (dev branch by default) from the Subversion repository. 

<div class="code" style="font-family: monospace;">
svn checkout http://opennero.googlecode.com/svn/trunk/
</div>

<li> Run cmake to generate Makefiles and/or IDE files
	<ol>
	<li> Either in the directory you checked out: 

<div class="code" style="font-family: monospace;">
cmake .
</div>

	<li> Or in a fresh directory (preferred): 

<div class="code" style="font-family: monospace;">
cmake ../PATH_TO_YOUR_CHECKOUT_FOLDER
</div>

	<li> See 

<div class="code" style="font-family: monospace;">
cmake --help
</div> 

for generating for your IDE

	<li> To build the Debug version of the code, pass the parameter ""-DCMAKE_BUILD_TYPE=Debug"" to cmake
	</ol>

<li> Perform the build using make or your selected IDE (such as Eclipse CDT)

	Use make for the basic build: 

<div class="code" style="font-family: monospace;">
make
</div>

<li> The build products should appear in the <strong>dist</strong> directory. Note that the scripts and data are copied to this directory, so either modify them there (and copy back to commit) or modify them in the original location (build) and rerun make to update the copy.
</ol>
 */
