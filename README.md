# ⚗️ Alchemist's Potion Lab

An interactive 3D alchemy laboratory featuring physically simulated liquid inside a glass flask, an atmospheric wooden workbench, procedurally animated candle flames, and dynamic lighting.

The project is developed using **C++14**, **OpenGL 3.3 Core Profile**, and **GLSL shaders**.

---

## ✨ Features

### 1. 🪵 Alchemical Oak Table & Environment

- **Massive wooden tabletop** featuring procedural wood grain and aged dark oak growth rings.
- **Detailed table structure:** four sturdy wooden legs, longitudinal and transverse support beams.
- **Laboratory environment:** stone-tiled floor and an ancient alchemist's dungeon wall.
- **Antique parchment:** an old alchemical manuscript with handwritten potion recipes resting on the table.
- **Magical rune circle:** an illuminated transmutation circle embedded into the tabletop beneath the flask. Its glow pulses with the potion's magic and reacts to flask movement.

### 2. 🕯️ Atmospheric Candles & Living Flames

- **Two candle arrangements:**
  - Left side: three wax candles of different heights on an antique brass plate with melted wax details.
  - Right side: two slender candles on a metal candle holder.
- **Wax material:** Blinn-Phong shading with warm subsurface scattering, ivory-colored wax, and charred wicks.
- **Procedural flame animation (`flame.vert` / `flame.frag`):**
  - Teardrop-shaped 3D flame geometry.
  - Independent harmonic flickering and swaying for each candle.
  - Temperature-based color gradient: blue-white core → golden flame → dark orange tip.
- **Glow Halo:** soft, additive radial glow surrounding each flame, creating realistic light scattering in the dark environment.
- **Candle control:** toggle all candles on or off using the `[L]` key.

### 3. 💡 Dynamic Lighting (Point Lights)

- **Light Source 1 – Left Candle Cluster:** warm amber illumination with nonlinear procedural flickering.
- **Light Source 2 – Right Candle Cluster:** golden candlelight illuminating the surrounding environment.
- **Light Source 3 – Magical Elixir:** soft magical illumination emitted directly from the flask, lighting the wooden table, runes, and flask itself according to the potion's current color.
- **Glass reflections:** the Erlenmeyer flask reflects the dancing candle flames, enhancing depth and visual realism.

### 4. 🧪 Liquid Physics (Slosh Physics)

- Simulation of damped harmonic oscillations of the liquid's free surface in response to acceleration and flask movement.
- Wave disturbances and surface ripples when shaking the flask.
- Rising micro-bubbles and a glowing meniscus along the glass boundary.
- Interactive flask manipulation using the mouse, allowing movement across the table and through the air.
- **Six predefined alchemical potion recipes**, each with a unique color and magical appearance:
  - Emerald Elixir
  - Ruby Elixir
  - Sapphire Elixir
  - Amethyst Elixir
  - Golden Elixir
  - Ice Elixir
- Smooth color transitions when switching between potions.

---

## 🎮 Controls

| Key / Action | Function |
|---|---|
| **Left Mouse Button (Hold & Drag)** | Grab and move the flask. The liquid reacts with physically simulated sloshing. |
| **Right Mouse Button (Hold & Drag)** | Rotate the 3D camera around the scene. |
| **Mouse Wheel** | Zoom in and out. |
| **Space** | Shake and mix the potion. |
| **[1] – [6]** | Switch between potion types and colors (Life Elixir, Dragon's Breath, Mana Essence, etc.). |
| **[C]** | Automatically cycle through potion colors. |
| **[L]** | Toggle candles on and off. |
| **[+] / [-]** | Increase or decrease the liquid level inside the flask. |
| **[R]** | Reset the flask to the center of the table and calm the liquid. |
| **[V]** | Reset the camera to its default view. |
| **[H]** | Display the control instructions in the console. |
| **[ESC]** | Exit the application. |

---

## 🛠️ Build & Run

### Requirements

- Windows 10 / 11
- CMake 3.10 or newer
- Visual Studio with MSVC and C++14 support (or newer)

### Quick Start

1. Run `build.bat` to automatically configure and build the project in **Release** mode using CMake.
2. Run `run.bat` to launch the application.

---

## 🧰 Technologies

- **C++14** – Core application logic and physics simulation.
- **OpenGL 3.3 Core Profile** – 3D rendering pipeline.
- **GLSL** – Custom vertex and fragment shaders.
- **CMake** – Project configuration and build system.
- **Visual Studio / MSVC** – Compilation environment.

---

*Enter the world of alchemy, experiment with magical elixirs, and bring your own potion laboratory to life!* ⚗️
