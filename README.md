# OpenGLVoxelEngine
A voxel engine written in c++ and OpenGL with GLFW and GLAD. 

![Terrain Image](./Images/Terrain.png)


# Build - VSCode (Windows)

Run the following script:
```cmd
.\Project_Setup.bat
```

Once all of the dependencies have been installed, use the cmake tools extension to build the project

## CMake Tools
## Select a kit:
This project is compiled with the Visual Studio Community 2022 Release - amd64 kit. To select a kit (or a similar one) enter the command "CMake: Select a Kit".

Then run "CMake: Build" and finally "CMake: Debug";


# Developer Notes:

## TODO:
* ### Optimize chunk mesh
    - Only render the top faces exposed to air
    - Have chunks talk to neighbors so touching voxels dont render inside faces

* ### Dynamically load and unload chunks
    - Currently not rendering chunks in view frustum
    - Can take this further by unloading chunks outside of render distance, deleting the buffer objects to save memory on the GPU
    - Need to reload the chunks when it gets inside the render distance,
    prioritizing chunks nearest the camera position (and within the view frustum).

* ### Infinite terrain generation
    - Building off of loading and unloading chunks within radius.
    - If radius surrounding camera position isn't filled with chunks, create chunk on Render Thread.
    - Update will then load the chunks.
    - Render will finalize buffer objects and then render the chunks. 