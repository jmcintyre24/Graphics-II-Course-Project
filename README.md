# Graphics-II-Course-Project
Graphics II Project Repository

## Overview
This is a DirectX11 rendering API created for Project & Portfolio IV (*Graphics-II*) using Gateware libraries. (Which are written & Maintained @ Full Sail University, License's in project.).  
***CMake***(**VER.** *3.16+*) is required to build the project, *though there is an executable in the MAIN\Build folder.*

***MAIN*** is the newest project itself, contains a **BUILD folder with two executables (With normal mapping or without normal mapping.)**. [The Build\README.txt reiterates what's stated here!]  
#### Cubes are used to represent the lights that have been implemented.
The cube inwards by the center of the mesh is the point light, the cube farthest away from the mesh is the directional light.
Along with that, the cube that is the directional light has the unique pixel shader applied to it.

## Controls:
- **Holding right click, move mouse** around to look around.
- **C** travels downwards.
- **Space** travels upwards.
- **WASD** for general strafing/movement.
- **Q & E** for rotating the camera quickly left or right.
- **Z** toggles the ability to move the directional light.
- **Holding left click (AFTER PRESSING Z), dragging** moves the directional light horizontally.
- **J & L** spins the mesh.
- **Left Shift & Control** zooms in and back out. (Shift > Inwards, Control > Outwards)
- **R** resets camera zoom.

## Features (WIP):
- [x] Complex Mesh Loading with Obj2Header
- [x] Textures on Complex Mesh
- [x] Directional Lighting on Complex Mesh
- [x] Point Light on Complex Mesh (With Range Attenuation)
- [x] Two Different Functional Lights on the same drawn geometry. (Point Light & Directional Lighting)
- [x] Dynamic Change in direction of directional light & dynamic Change in position of positional light.
- [x] Normal Mapping on a full 3D complex mesh [StoneHenge].
- [x] Unique Pixel Shader using relative position, time, and a wave. Causes whatever mesh/object that is using that pixel shader to pulse and change colors based on the X, Z, and time.
- [x] Proceduarly Generated 3D Grid.
- [x] Stabilized FPS fly-through style camera <--- Fixed Stabilization Issue
- [x] Infinite Skybox
- [x] Simple Camera Zoom


