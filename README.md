# Maze Escape 3D — Computer Graphics Project

A 3D first-person OpenGL labyrinth exploration game featuring procedural maze generation, dynamic lighting, glowing orb pickups, fog-of-war minimap, and portal exit mechanics.

---

## 🎮 Gameplay & Objectives

- **Goal**: Navigate through procedural 3D labyrinths, collect all floating glowing orbs to unlock the portal, and reach the portal before the countdown timer expires!
- **Level Progression**: Every level the maze grid dynamically scales in size, adding more orbs and challenging routes.

---

## 🕹️ Controls

| Key | Action |
| :--- | :--- |
| **W** / **Up Arrow** | Move Forward |
| **S** / **Down Arrow** | Move Backward |
| **A** / **Left Arrow** | Turn Left |
| **D** / **Right Arrow** | Turn Right |
| **Q** / **E** | Strafe Left / Right |
| **Enter** | Start Game / Next Level / Retry |
| **M** | Toggle Minimap (Fog of War) |
| **P** | Pause / Resume |
| **R** | Restart Level |
| **Esc** | Exit Game |

---

## 💻 Computer Graphics (CG) Concepts Implemented

- **Procedural Maze Generation**: Iterative recursive backtracker on an NxN cell grid with loop carving.
- **First-Person 3D Camera**: Perspective projection via `gluLookAt` with smooth yaw rotation and head-bobbing oscillation.
- **Procedural Textures**: Handcrafted procedural brick wall patterns (with per-brick shading and mortar) and tiled stone floors with mipmapping.
- **Dynamic Lighting**: Mobile torch light attached to player eye space (`GL_LIGHT0`) plus exit portal point light (`GL_LIGHT1`).
- **Depth Cueing (Fog)**: Exponential squared fog (`GL_EXP2`) for depth perception.
- **Display Lists**: Static geometry compiled into OpenGL display lists (`glCallList`) for smooth 60 FPS performance.
- **Collision & Wall Sliding**: Axis-separated bounding-box collision detection allowing seamless sliding along walls.
- **2D HUD & Fog-of-War Minimap**: Orthographic projection overlay (`gluOrtho2D`) with alpha blending, crosshair, dynamic status toasts, and explored minimap revelation.

---

## 🚀 How to Run

### Method 1: Double-Click
Run **`run.bat`** or double-click **`maze.exe`**.

### Method 2: Command Line
```powershell
.\maze.exe
```

### Method 3: Rebuild and Run
```cmd
build.bat
```
Or directly with MinGW `g++`:
```powershell
g++ -O2 -Iinclude game.cpp -o maze.exe -lopengl32 -lglu32 -lgdi32 -lwinmm
```

---

## 🌐 Web Version (Optional)
A browser-based Three.js version is also available in the `web/` folder. Open `web/index.html` in any browser to play without compiling.
