# Credits & Asset Attributions

All 3D geometry meshes, procedural textures, GLSL shaders, and bitmap font rendering in **Alchemist's Potion Lab** were created procedurally in C++14 and GLSL 330 Core by the project team, using only standard course-approved OpenGL helper libraries bundled in `deps/`.

---

## 1. Third-Party Libraries

| Library | Author / Organization | Link | License | Usage in Project |
|---|---|---|---|---|
| **FreeGLUT** | The FreeGLUT Project | https://freeglut.sourceforge.net/ | X-Consortium / MIT License | Cross-platform OpenGL 3.3 Core Profile window creation, input callbacks (`keyboard`, `mouse`), and event loop |
| **GLEW** (OpenGL Extension Wrangler) | Milan Ikits, Marcelo Magallon, Nigel Stewart | https://glew.sourceforge.net/ | Modified BSD / MIT License | Runtime loading of modern OpenGL 3.3+ core functions (VAOs, VBOs, shaders, `glDrawElementsInstanced`, `glVertexAttribDivisor`) |
| **OpenGL 3.3 Core Profile** | Khronos Group | https://www.opengl.org/ | Khronos Specification | Hardware-accelerated 3D rendering API |

---

## 2. Procedural 3D Models & Textures (Original Team Work)

No external `.obj`, `.gltf`, or `.png`/`.jpg` asset files are imported. Every mesh and texture is procedurally generated at startup:

| Asset Category | Implementation File | Author | License |
|---|---|---|---|
| **3D Procedural Geometry** (Erlenmeyer flask, liquid volume, slosh surface disk, reagent vials, oak workbench, stone dungeon room, spellbooks, mortar & pestle, apothecary shelf, torus astrolabe rings, teardrop candle flames) | `src/Mesh.cpp`, `src/LabEnvironment.cpp` | Project Team (Diyar Amangeldiyev, Balnur Sailaubek, Ramazan Nagimov) | Original Coursework (MIT) |
| **Procedural Mipmapped Textures** (Antique oak wood grain, stone brick masonry, inked parchment scroll, embossed leather book cover, moonlit gothic window, mortar stone, rotating arcane rune circle, soft Gaussian particle sprite) | `src/Texture.cpp` | Project Team | Original Coursework (MIT) |
| **5×7 Monochrome ASCII Bitmap Font Table** | `include/BitmapFont.h` | Classic standard 5×7 ASCII pixel glyph encodings authored directly in header | Public Domain / Original |
| **Custom GLSL 330 Core Shaders** (9 `.vert`/`.frag` shader pairs: `standard`, `flask_liquid`, `flask_glass`, `cork`, `flame`, `rune`, `glow`, `particle`, `ui`) | `shaders/*` | Project Team | Original Coursework (MIT) |

---

## 3. Educational References & Conceptual Citations

| Resource | Author | Link | License / Usage |
|---|---|---|---|
| **LearnOpenGL** (Coordinate Systems, Basic Lighting / Blinn-Phong, Blending, Instancing, Text Rendering concepts) | Joey de Vries | https://learnopengl.com/ | CC BY-NC 4.0 — Referenced conceptually for OpenGL 3.3 Core Profile pipeline conventions, Blinn-Phong attenuation formulas, back-to-front transparency ordering, and `glVertexAttribDivisor` instancing. All C++ and GLSL code was written and adapted specifically for this project without copying tutorial files verbatim. |
| **Course Labs & Lecture Notes** (Computer Graphics Fundamentals) | Nazira Shaimerdenova, Astana IT University | Course LMS (Moodle) | Course foundation for FreeGLUT + GLEW + CMake Windows build setup and core shading concepts. |
