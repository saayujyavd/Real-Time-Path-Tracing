# Real Time Path Tracing

A real-time path tracer built in C++ using NVidia's Compute Unified Device Architecture (CUDA), Open Graphics Library (OpenGL), and Immediate Mode Graphical User Interface (ImGui).
Based on "Ray Tracing in One Weekend" by Peter Shirley, Trevor David Black, Steve Hollasch.

## Features
- GPU-accelerated path tracing via CUDA
- Real-time viewport via OpenGL Pixel Buffer Object (PBO)
- Interactive camera (WASD + mouse look)
- ImGui controls for samples, depth, Field of View (FOV), defocus blur (Depth of Field)

## Hardware Requirement
- NVIDIA GPU (sm_86 or other arch flags)

## Software Requirements
- CUDA Toolkit
- GLFW 3.4
- GLEW
- ImGui

## Build
Edit paths in 'build.bat' to match your system, then run.

## Controls
| Key 			| Action 	|
|-----------------------|---------------|
| W/A/S/D 		| Move camera 	|
| Right Mouse + Drag 	| Look around 	|
| Escape 		| Quit 		|
