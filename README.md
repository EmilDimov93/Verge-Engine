# <img width="40" height="32" alt="Logo" src="https://github.com/user-attachments/assets/b02d83e3-3902-4222-a22a-70c3ce1ec1d8" /> Verge Engine

![Language](https://img.shields.io/badge/C%2B%2B-23-00599C)
![Graphics API](https://img.shields.io/badge/Vulkan-1.3-A41E22)
![License](https://img.shields.io/badge/license-Apache--2.0-purple)

Verge Engine is a Vulkan-based C++ engine for realistic vehicle simulations

<img width="1265" height="543" alt="verge_showcase_mtn" src="https://github.com/user-attachments/assets/ba0ba5de-2530-407e-858e-61a9b25c582e" />

**Note:** Verge Engine is under active development and not yet usable as a finished product. To experiment with the current state, see [run.ps1](run.ps1)

## Features
#### Rendering with Vulkan
   - Dynamic shadows
   - Lighting (Blinn-Phong)
   - Real-time multi-threaded model loading (`.glb`, `.obj`)
   - Textures with mipmaps
   - Post-effects (FXAA, dithering, vignette)
   - UI
#### Vehicle physics
   - User-configurable engine, drivetrain and gearbox simulation
   - Animated wheels (steering, spin, camber, suspension)
   - Terrain collisions
#### Audio with miniaudio
   - 3D spatial audio
   - Layered engine audio
#### Input system with GLFW
   - Keyboard, gamepad and steering wheel support
   - Axis dead-zone handling and per-input smoothing
#### World
   - Heightmap terrain with custom surface types
   - Props
   - Triggers

## Dependencies
- Vulkan (Rendering)
- GLFW (Window and input)
- [Miniaudio](ext/miniaudio) (Audio)
- [glm](ext/glm) (Math)
- [stb_image](ext/stb_image) (Texture loading)
- [cgltf](ext/cgltf) (.glb model loading)
