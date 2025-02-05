# About this project
In its current state (as of February 2025), this project features a custom procedural terrain generation using raylib.
In addition simple terrain deformation tools are present as well.

The project is in active development and not stable. Bugs will occur.

This project has no particular goal, other than serving as a place for me to learn and implement concepts, that I find interesting.

The code inside the folders (and their subdirectories) `inc/`, `src/` and `raylibBackend/` is written by me.

# Usage
Either use the precompiled binary for windows or build yourself using CMake.
To do build using cmake, go into the root folder and execute `cmake -B build -G "Visual Studio 17 2022"
 && cmake --build build --config Debug`

After that you can find the executable under `build/Debug/Terraining.exe`.

# Controls
Once loaded, use ALT to toggle between view and mouse mode.
You can use WASD, Space and CTRL to move around and SHIFT will make you faster.
To deform the terrain, check the "Check mouse" Checkbox and use the left mouse button to start.