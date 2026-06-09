# <img width="40" height="32" alt="Logo" src="https://github.com/user-attachments/assets/b02d83e3-3902-4222-a22a-70c3ce1ec1d8" /> Verge Engine

Verge Engine is a Vulkan-based C++ engine for realistic vehicle simulations

**Note:** Verge Engine is under active development and not yet usable as a finished product. To experiment with the current state, see [run.ps1](run.ps1)

<img width="1268" height="737" alt="verge_showcase" src="https://github.com/user-attachments/assets/1a84d626-d1b0-4b5b-9b79-5b5579c4c884" />

## Features
#### Rendering with Vulkan
   - Dynamic shadows
   - Lighting (Blinn-Phong)
   - UI
   - Textures with mipmaps
   - Real-time multi-threaded model loading
   - Post-effects
#### Vehicle physics
   - User-configurable engine and gearbox simulation
   - Animated wheels
   - Collisions
#### Audio with miniaudio
   - 3D spatial audio
   - Layered engine audio
#### Input system with GLFW
   - Keyboard, gamepad and steering wheel support
#### World
   - Heightmap terrain
   - Props
   - Triggers

## Dependencies
- Vulkan (Rendering)
- GLFW (Window and input)
- [Miniaudio](ext/miniaudio) (Audio)
- [GLM](ext/glm) (Math)
- [stb_image](ext/stb_image) (Texture loading)
