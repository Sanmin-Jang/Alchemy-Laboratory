# Alchemist's Potion Lab

**Course:** Computer Graphics Fundamentals — Astana IT University  
**Instructor:** Nazira Shaimerdenova  
**Topic:** #15 — Alchemist's Potion Lab (★★)  
**API & Language:** C++14, OpenGL 3.3 Core Profile, GLSL 330 Core

An atmospheric 3D interactive alchemy laboratory game built from scratch in modern OpenGL 3.3 Core Profile. Players start from an animated 3D Main Menu, walk freely around a candlelit medieval stone dungeon laboratory, pour 3 magical reagents into a central Erlenmeyer flask with damped-harmonic liquid slosh physics, and brew 3 distinct potion combinations with dramatic real-time visual effects.

---

## Team Members & Contributions

| Team Member | Student ID | Primary Responsibilities |
|---|---|---|
| **Diyar Amangeldiyev** | 241513 | Core OpenGL 3.3 pipeline, `Shader` / `Mesh` / `math3d` architecture, damped-harmonic `SloshPhysics`, multi-pass Fresnel glass & liquid shaders, build system (`CMakeLists.txt`) |
| **Balnur Sailaubek** | 240532 | Atmospheric `LabEnvironment` (stone dungeon room, gothic window, oak workbench, leather spellbooks, wall shelf, 3-level hanging brass astrolabe hierarchy), procedural mipmapped `Texture` generators, multi-light Blinn-Phong & dungeon fog shading |
| **Ramazan Nagimov** | 242394 | Interactive game loop & `Camera` (FPS walking with collision + Orbit mode), `PotionSystem` (3 ingredients, 3 combinations & reactions), GPU-instanced `ParticleSystem` (bubbles, steam, explosion, arcane spiral, frost mist, dust), Core-Profile 2D `UIRenderer` (Menu, HUD, FPS overlay, Grimoire help) |

---

## Features & Course Requirements Mapping

- **R1 — Modern Pipeline:** Pure OpenGL 3.3 Core Profile (`GLUT_CORE_PROFILE | GLUT_FORWARD_COMPATIBLE`), VAO/VBO/EBO meshes, 9 separate GLSL `.vert`/`.frag` shader pairs, and a pure Core-Profile 2D UI renderer (zero fixed-function calls).
- **R2 — Scene Setup:** 15+ distinct 3D objects in an enclosed stone dungeon laboratory (oak workbench with aprons & legs, main Erlenmeyer flask + cork, 3 glowing reagent vials, 5 candles on 2 antique brass saucers with melted wax drips, 3 stacked gold-embossed leather spellbooks, recipe parchment scroll, mortar & pestle, apothecary wall shelf with colored jars, moonlit gothic arched window, timber ceiling beams, stone pillars, and hanging brass astrolabe).
- **R3 — Transformation Pipeline & Hierarchies:** Custom column-major `Mat4` MVP & normal matrices (`math3d.h`) with multiple working parent–child transformation hierarchies:
  1. **3-Level Hanging Brass Astrolabe:** `CeilingMount -> OuterBrassRing (Y-rot) -> InnerBrassRing (X/Z-rot) -> ArcaneCrystalCore (Y-rot & emissive resonance)`
  2. **Main Flask Hierarchy:** `FlaskRoot -> LiquidVolume + GlassShell + CorkStopper` (cork detaches & launches dynamically on Explosion)
  3. **Animated Ingredient Vials:** `BottleRoot (lift/tilt arc) -> LiquidCylinder + Cork + FresnelGlass`
  4. **Lab Props:** `Mortar -> Pestle`, `WallShelf -> ApothecaryJars`
- **R4 — Lighting & Shading:** Per-fragment Blinn-Phong lighting with ambient, diffuse, specular, and quadratic distance attenuation across **4 dynamic point lights** (Left flickering candle cluster, Right flickering candle cluster, dynamic color-shifting Glowing Potion light, and cool Gothic Window moonlight) plus exponential atmospheric dungeon fog.
- **R5 — Procedural Texturing & Mipmapping:** Procedural RGBA textures with proper UVs and `glGenerateMipmap` (`GL_LINEAR_MIPMAP_LINEAR`) for antique oak wood grain, stone masonry, inked recipe parchment, embossed book leather, moonlit gothic window, mortar stone, rotating arcane rune circle, soft Gaussian particle sprite, and crisp 5×7 bitmap UI font atlas.
- **R6 — Interactivity & Game Loop:** Full `MAIN_MENU -> PLAYING -> PAUSED / RECIPES` game state machine, smooth frame-rate-independent FPS walking (`WASD` + AABB room/table collision) + toggleable Orbit Camera (`V`), ray-picked clickable reagent bottles, interactive flask grabbing & sloshing, candle toggle (`L`), and on-screen Grimoire help overlay (`H`).
- **R7 — Real-Time Performance:** GPU-instanced 3D particle rendering (`glDrawElementsInstanced` + `glVertexAttribDivisor`) achieving **2,700+ FPS** at 1280×720 on an NVIDIA RTX 5060 Ti (well above 60 FPS on standard TA hardware), with an always-visible on-screen FPS counter and window-title FPS metric.
- **R8 — Reproducible Build:** Self-contained dependencies (`deps/` includes FreeGLUT and GLEW headers, libs, and DLLs) built identically to course assignments via CMake and `build.bat`.

### Stretch Goals Implemented (Topic 15)
1. **3D Bubbles, Steam & Reaction Particles (GPU-Instanced):** Continuous rising bubbles inside the flask, translucent steam wisps from the flask neck, ambient golden laboratory dust motes, and full reaction particle bursts (Explosion embers/smoke, Arcane helical sparks, Glacial cryo-mist).
2. **Glowing Rotating Magic Circle on Recipe Brew:** An arcane transmutation sigil beneath the main flask that rotates faster, pulses radially, and surges in color-matched luminosity when a valid potion recipe is brewed.
3. **Semi-Transparent Fresnel Glass:** Two-pass back-face/front-face culled glass rendering with Schlick-like Fresnel rim reflectance and dynamic candle glints on both the main Erlenmeyer flask and all 3 ingredient vials.

### The 3 Potion Combinations & Visual Effects
Mix any **2 ingredients** (`[1]`, `[2]`, `[3]` or left-click the glowing vials on the workbench), then press **`[E]`** or **`[ENTER]`** to brew:

| Combination | Potion Name | Visual Effect |
|---|---|---|
| **Crimson Ember `[1]` + Venom Leaf `[3]`** | **Volatile Inferno** | **Alchemical Explosion!** Molten orange boiling brew, cork pops and tumbles through the air, expanding glowing shockwave ring across the workbench, camera shake, bright orange flash, and 200+ fiery embers & billowing soot smoke particles. |
| **Crimson Ember `[1]` + Frost Dew `[2]`** | **Astral Starlight** | **Arcane Nova (Wow Moment)!** Radiant magenta-violet transmutation; the glowing magic rune circle surges & spins rapidly, ascending helical violet-gold starlight particles spiral upward, the overhead brass astrolabe accelerates, and the finished potion intensely illuminates the entire dark laboratory. |
| **Frost Dew `[2]` + Venom Leaf `[3]`** | **Glacial Mist** | **Cryo-Freeze!** Shimmering ice-cyan elixir; swirling frost crystals and dense sinking dry-ice cryo vapor billow over the workbench while candle flames shiver to an icy cyan tint and cool frost fog envelops the dungeon room. |

---

## Controls

| Key / Input | Action |
|---|---|
| **`W` / `A` / `S` / `D`** | Walk smoothly around the laboratory (FPS Walk Mode with table & wall collision) |
| **`Right Mouse Button (Drag)`** | Rotate camera view (Look in FPS Mode / Orbit in Inspect Mode) |
| **`F`** | Toggle Free Mouse Look in FPS Mode |
| **`V`** | Switch between **FPS Walk Mode** and **Workbench Orbit Mode** |
| **`Mouse Scroll Wheel`** | Zoom camera (Orbit Mode) / Adjust eye elevation (FPS Mode) |
| **`1` / `2` / `3`** or **Left-Click Vial** | Pour **Crimson Ember**, **Frost Dew**, or **Venom Leaf** into the main flask |
| **`E` / `Enter`** | **Brew** the 2-ingredient mixture and trigger its magical visual effect |
| **`Left Mouse Button (Drag Flask)`** | Grab and shake the main Erlenmeyer flask (real-time slosh physics) |
| **`Space`** | Stir / agitate the potion liquid |
| **`+` / `-`** | Increase / decrease liquid fill level |
| **`L`** | Toggle flickering candle lights ON / OFF (try OFF during *Astral Starlight*!) |
| **`R`** | Clear flask & reset position to center |
| **`H`** | Open / close on-screen **Alchemist's Grimoire** (Recipes & Controls) |
| **`ESC`** | Open Pause Menu / Back |

---

## Build & Run Instructions

### Prerequisites
- Windows 10 / 11 (x64)
- CMake 3.10 or newer
- Visual Studio 2019 / 2022 (MSVC C++14 toolset)
- OpenGL 3.3+ compatible GPU driver

All third-party libraries (`FreeGLUT`, `GLEW`) are already bundled in `deps/` — no external installation is required.

### Option 1: One-Click Batch Scripts (Recommended)
```bat
build.bat
run.bat
```

### Option 2: Manual CMake Commands
```powershell
cmake -S . -B build
cmake --build build --config Release
cd build\Release
.\AlchemistPotionLab.exe
```

To run a quick automated startup and 20-frame sanity test:
```powershell
.\AlchemistPotionLab.exe --test
```

---

## Project Structure

```text
Alchemy-Laboratory-main/
├── CMakeLists.txt          # CMake build configuration & post-build asset copy
├── build.bat / run.bat     # Convenience Windows build and launch scripts
├── README.md               # Project documentation, controls, roles & AI Use Note
├── CREDITS.md              # Third-party library, tutorial & asset attributions
├── DEMO_VIDEO_SCRIPT.md    # 4:30 demo video narration & shot script
├── deps/                   # Bundled FreeGLUT & GLEW headers, libs, and DLLs
├── include/                # Modular C++ headers (each strictly < 400 lines)
│   ├── BitmapFont.h        # 5x7 ASCII bitmap font table for Core-Profile UI
│   ├── Camera.h            # FPS walking, Orbit inspect & Menu cinematic camera
│   ├── LabEnvironment.h    # Stone room, workbench, books, shelf, astrolabe, candles
│   ├── Mesh.h              # VAO/VBO/EBO mesh wrapper & procedural shape builders
│   ├── ParticleSystem.h    # GPU-instanced 3D particle system
│   ├── PotionSystem.h      # 3-ingredient mixing logic, 3 recipes & pouring animations
│   ├── Scene.h             # Top-level 3D scene orchestrator & multi-light setup
│   ├── Shader.h            # GLSL shader compiler & uniform helpers
│   ├── SloshPhysics.h      # 2D damped harmonic liquid surface oscillator
│   ├── Texture.h           # Procedural mipmapped texture generators & BMP loader
│   ├── UIRenderer.h        # Pure OpenGL 3.3 Core Profile 2D menus, HUD & FPS badge
│   └── math3d.h            # Custom Vec2/Vec3/Vec4 and column-major Mat4 math library
├── src/                    # Modular C++ implementations (all files < 500 lines)
└── shaders/                # 9 separate GLSL 330 Core .vert/.frag shader pairs
```

---

## AI Use Note

AI assistance (Gemini) was employed to expand upon the team's initial draft notes, refine the English academic phrasing, format the technical documentation in the project README.md, CREDITS.md, and structure the laboratory report according to the university's prescribed template guidelines. In addition, AI assistance was used during the technical implementation stage to generate the initial prototype of the damped-harmonic liquid slosh physics (SloshPhysics), assist in designing the 3-level hanging brass astrolabe transformation hierarchy and dungeon laboratory props (LabEnvironment), and draft the initial GPU-instanced particle billboard shader (particle.vert and particle.frag). All AI-generated code snippets and prototypes were subsequently reviewed, substantially rewritten, and refined by the team members to fit the modular C++14 and OpenGL 3.3 Core Profile architecture.
