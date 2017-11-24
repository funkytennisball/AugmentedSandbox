[![Augmented Sandbox](https://img.youtube.com/vi/R3bPysO-EAI/0.jpg)](https://www.youtube.com/watch?v=R3bPysO-EAI)

## Description
* Mesh reconstruction from Kinect point cloud feed including noise removal, smoothening and triangulation 
* Physics/Fluid simulation using modified height field fluid simulation, see [Balint etc](http://www.balintmiklos.com/layered_water.pdf)
* Rendering using GL shaders

## Project Structure
Project runs on 3 threads Main thread

### Main Thread/Core
Main thread (SandboxCore.cpp), inits subthreads, pipeline and starts GUI here 
``` c++
// start fetching data
m_DepthFetcher.start();
m_Physics.start();

// open topdown view
CreateTopDownViewer();
```

### Depth Fetcher
DepthFetcher.cpp deals with point clouds, passes mesh as Triangulation.h
``` c++
TerrainPtr& e_Terrain;
std::shared_ptr<Triangulation>& e_Triangulation;
```

### Physics Thread
PhysicsThread.cpp deals with physics simulation, updates simulation data as World.h 
``` c+++
std::array<float, cArrayLength> waterHeight;
std::array<float, cArrayLength> horVelocity;
std::array<float, cArrayLength> vertVelocity;
```

## Requirements
* Microsoft Kinect
* CMake
* GPU capable of running OpenGL 2.x or higher
* Sandbox
* Projector
