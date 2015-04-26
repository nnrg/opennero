## Introduction ##

In this exercise, you will go through the steps required to create a Mod consisting of a flat terrain surrounded by four walls and a simple environment with an agent that can move around. This exercise can be done either with the [working copy of the source](BuildingOpenNero.md) or with one of the [binary distributions](DownloadingOpenNero.md).

  * If working with source, use the `mods` directory and update the application build to copy your files into the product folder `dist`.
  * If working with a binary distribution, use the same folder as the executable on Windows and Linux, or `OpenNERO.app/Contents/Resources` folder on Mac OS X (right click on the app to "Show Package Contents".

## Creating a Mod ##

  * Create a new sub-folder in the mods directory you are using. For the rest of this discussion we will assume this folder is called TESTMOD, but it can be anything you want.
  * Create two (empty) files in TESTMOD: **`main.py`** and **`__init__.py`** (note the double underscores in front of and after init in the filename).
  * In **main.py**, define the `ModMain` function:
```
def ModMain():
    print "Hello, Mod!"
```
  * This is the function that will get called when you load your mod.
  * If using source, update the build
  * Run the application

You should see a button appear for TESTMOD in the Hub menu. When you press this button, you should see an empty screen and the output log (usually nero\_log.txt) should contain 'Hello, Mod!'. Congratulations, you've added a mod!  Press the window close button to quit the application.

Now let's make it more useful.

## nero\_log.txt ##

If you launch OpenNERO from the command line, you can see the OpenNERO log messages printed on the terminal as the application is running.  To do so on Linux, open a terminal, change the directory to where you installed OpenNERO, and type:

```
./OpenNERO
```

You can also find the OpenNERO log in a file at the following location:

Linux or Mac: ~/.opennero/nero\_log.txt
Windows: "AppData\Local\OpenNERO\nero\_log.txt" or "Local Settings\Application Data\OpenNERO\nero\_log.txt" depending on the version of Windows you have.

## Adding objects ##

Each new type of object in OpenNERO is defined in an XML file called a Template. Here is an example (which is just a cube):
```
<Template>
  <Audio>
  </Audio>
  <Render>
    <MaterialType>solid</MaterialType>
    <MaterialFlagLighting>false</MaterialFlagLighting>
    <AniMesh>data/cube/cube.obj</AniMesh>
    <Scale>1 1 1</Scale>
    <Rotation>0 0 0</Rotation>
  </Render>
</Template>
```

Create this file in `TESTMOD/data/cube/Cube.xml`

Here are some things about this file:
  * Top level sections, like Audio, Render and AI (which we will add later) describe different aspects of the entity.
  * Render part describes how the object looks. In particular, it tells OpenNERO that:
    * the object is made of solid material
    * it is not a light source
    * it is based on the (possibly animated) mesh in cube/cube.obj
    * it defaults to the X-Y-Z scale of 1x1x1 (relative to mesh dimensions)
    * it is initially not rotated from the mesh (using around-X, around-Y, around-Z [Euler Angle](http://en.wikipedia.org/wiki/Euler_angles) in degrees)

Now we need to get the [obj file](http://en.wikipedia.org/wiki/Obj) that our XML file refers to. There are several ways to get such a file - make a new model in a 3D modeling tool and export it, refer to one of the common models in OpenNERO, or copy the common model into the local directory (if you need to modify it for your mod). OpenNERO already comes with a cube obj file, so we can just refer to it with:
```
<AniMesh>data/shapes/cube/cube.obj</AniMesh>
```

Or, you can copy [mods/common/data/shapes/cube/cube.obj](http://opennero.googlecode.com/svn/trunk/mods/common/data/shapes/cube/cube.obj) to `TESTMOD/data/cube/`

Now that you have the template `data/cube/Cube.xml`, you should change your `main.py` to actually add a cube into the world. The new file should look something like this:

```
from OpenNero import * # import the OpenNERO C++ API, such as Vector3f
from common import * # import commonly used OpenNERO Python API, such as addObject

def ModMain():
    print "Hello, Mod!"
    addObject("data/cube/Cube.xml", Vector3f(0,0,0)) # add a default-looking cube at the center of the world
```

If by the end of this you have something that looks like this:

<a href='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Cube.png'><img src='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Cube.png' height='300' width='400'></img></a>

Then you were successful, and you are looking at (one side of) a cube.

## Adding a camera ##

You are looking at the cube at a flat angle. If you wanted to see it's 3-dimensionality, you could try adding the following code:

```
    camRotateSpeed = 10
    camMoveSpeed   = 500
    camZoomSpeed   = 50
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    cam.setPosition(Vector3f(5, 5, 5))
    cam.setTarget(Vector3f(0, 0, 1))
```

Adding a camera in this way will change your vantage point, where you are looking, and allow you to use the **A**, **S**, **W** and **D** keys to move around, **Q** and **E** keys to rotate, and **Z** and **C** keys (or mouse wheel) to zoom in or out.

## Adding a terrain and walls ##

Let us now make our Mod more interesting by adding a textured terrain and walls surrounding the terrain.  We can do this in the same way that we added the cube, i.e. by creating the files defining the new objects and then invoking the `addObject()` function in `main.py`.  OpenNERO already comes with the files for a flat grassy terrain and a brick wall, which we will use in this exercise.

Copy the terrain files [mods/common/data/terrain/GrassyTerrain.xml](http://opennero.googlecode.com/svn/trunk/mods/common/data/terrain/GrassyTerrain.xml), [mods/common/data/terrain/grass.jpg](http://opennero.googlecode.com/svn/trunk/mods/common/data/terrain/grass.jpg), and [mods/common/data/terrain/terrain-heightmap-flat.bmp](http://opennero.googlecode.com/svn/trunk/mods/common/data/terrain/terrain-heightmap-flat.bmp) to `TESTMOD/data/terrain/`

The XML template file for the grassy terrain looks like this:
```
<Template>
  <ObjectTemplate>
    <Name>Terrain</Name>
  </ObjectTemplate>
  <Render>
    <MaterialType>solid</MaterialType>
    <MaterialFlagLighting>false</MaterialFlagLighting>
    <Terrain>data/terrain/terrain-heightmap-flat.bmp</Terrain>
    <Texture0>data/terrain/grass.jpg</Texture0>    
    <ScaleTexture>10 10</ScaleTexture>
    <Scale>0.390625 0.390625 1</Scale>
  </Render>
</Template>
```

Notice that the `Render` section in this template is different from the template for the cube in a couple of ways:
  * It uses a [height map](http://en.wikipedia.org/wiki/Heightmap) file instead of a mesh object file to specify the geometry of the terrain.  Height maps are more convenient for specifying terrains.
  * It specifies an image file of a grassy region as the texture for the surface of the terrain (see [texture mapping](http://en.wikipedia.org/wiki/Texture_mapping) for details).

Add the terrain to your Mod with the following line of code to `main.py`:
```
    addObject("data/terrain/GrassyTerrain.xml", Vector3f(-50,-50,0)) # add terrain at the given X-Y-Z coordinates
```

Now you should get the following, with the cube at the center of the terrain (the bottom half of the cube is below the terrain):

<a href='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Terrain.png'><img src='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Terrain.png' height='300' width='400'></img></a>

Zoom out to see the edges of the terrain.  The next task is to add bounding walls to the four edges of the terrain.  Create `TESTMOD/data/wall/BrickWall.xml` with the following XML template for the wall:
```
<Template>
  <Audio>    
  </Audio>
  <Render>
    <MaterialType>solid</MaterialType>
    <MaterialFlagLighting>true</MaterialFlagLighting>
    <AniMesh>data/wall/BrickWall.obj</AniMesh>
    <CastsShadow>true</CastsShadow>
    <Scale>2 3 3</Scale>
  </Render>  
</Template>
```

Copy the wall object file [mods/common/data/shapes/wall/BrickWall.obj](http://opennero.googlecode.com/svn/trunk/mods/common/data/shapes/wall/BrickWall.obj) to `TESTMOD/data/wall/`.  The object file references a [material file](http://en.wikipedia.org/wiki/Material_Template_Library) describing the visual properties of the model.  So copy that file [mods/common/data/shapes/wall/BrickWall.mtl](http://opennero.googlecode.com/svn/trunk/mods/common/data/shapes/wall/BrickWall.mtl) and the texture file [mods/Maze/data/shapes/wall/bricks.JPG](http://opennero.googlecode.com/svn/trunk/mods/Maze/data/shapes/wall/bricks.JPG) to `TESTMOD/data/wall/` as well.

The same template file can be used multiple times with `addObject()`, each time using a different geometric transformation to create different walls.  In addition to the `Vector3f()` argument specifying the X-Y-Z translation of the object, `addObject()` can also take two more `Vector3f()` arguments specifying the X-Y-Z rotation and X-Y-Z scale of the object respectively.  For example, the code
```
    addObject("data/wall/BrickWall.xml", Vector3f(-50,0,0), Vector3f(0,0,90), Vector3f(2,1,1))
```
rotates the wall 90 degrees about the Z-axis, scales it 2 times the original size along the X-axis, and moves it to one edge of the terrain.

In this manner, add four walls enclosing the terrain on all four sides:

<a href='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Walls.png'><img src='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Walls.png' height='300' width='400'></img></a>

## Adding an environment ##

An environment in OpenNERO is where you can define the rules of the world, such as what happens when an agent reaches the end of the maze, or when two agents bump into each other, or if one of them vacuums up a crumb.  Therefore, we must specify the environment in an environment class for the agent to function correctly.  We will discuss the detailed contents of the environment class in another exercise; for this exercise, just copy the file [environment.py](http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/environment.py), which contains a simple `TestEnvironment` class, to your `TESTMOD` directory.

You must now add the following line of code to the top of `main.py` to access the `TestEnvironment` class:
```
from TESTMOD.environment import *
```

To set it as the current environment, add the following code to the `ModMain()` function of `main.py`:
```
    # create environment for the agent
    env = TestEnvironment()
    set_environment(env)
```

## Adding an agent ##

An agent in OpenNERO is an object consisting of two parts:
  1. A graphical rendering of the body of the agent, and
  1. The AI specifying the behavior of the agent

These two parts are configured using an XML template file as follows:
```
<Template>
	<Render>
		<MaterialType>solid</MaterialType>
		<MaterialFlagLighting>true</MaterialFlagLighting>
		<AniMesh>data/character/sydney.md2</AniMesh>
		<Texture0>data/character/sydney.bmp</Texture0>
		<CastsShadow>true</CastsShadow>
		<Scale>0.2 0.2 0.2</Scale>
	</Render>
	<AI>
		<Python agent="TestAgent()" />
	</AI>
</Template>
```

Copy this template to `TESTMOD/data/character/SydneyAgent.xml`.

The `Render` section in this file is similar to what we have seen previously.  The files referenced in this section can be obtained from [mods/common/data/shapes/character/sydney.md2](http://opennero.googlecode.com/svn/trunk/mods/common/data/shapes/character/sydney.md2) and [mods/common/data/shapes/character/sydney.bmp](http://opennero.googlecode.com/svn/trunk/mods/common/data/shapes/character/sydney.bmp).  Copy them to `TESTMOD/data/character/`.

The `AI` section specifies a Python class `TestAgent` that contains the behavior of the agent, e.g. what actions the agent takes in response to its sensor readings.  We will discuss the details of this class in another exercise.  A simple `TestAgent` class that ignores the agent's sensor readings and moves the agent in response to keyboard arrow keys is implemented in [agent.py](http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/agent.py).  Copy this file to your `TESTMOD` directory.

Now add the following line of code to the top of `main.py`
```
from TESTMOD.agent import *
```
and add the following code to the `ModMain()` function
```
    # create key binding to control the agent
    ioMap = createInputMapping()
    getSimContext().setInputMapping(ioMap)

    # enable AI and create the agent
    enable_ai()
    addObject("data/character/SydneyAgent.xml", Vector3f(0, 0, 4.5))
```

This will give you an agent that you can control using the arrow keys:

<a href='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Agent.png'><img src='http://opennero.googlecode.com/svn/wiki/AddingStuffExerciseFiles/Agent.png' height='300' width='400'></img></a>