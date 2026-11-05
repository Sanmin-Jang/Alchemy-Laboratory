#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cmath>

#include "math3d.h"
#include "Camera.h"
#include "Scene.h"

// Window configuration
int windowWidth = 1280;
int windowHeight = 720;

// Application core components
Camera camera;
Scene scene;

// Mouse tracking and dragging
int lastMouseX = 0;
int lastMouseY = 0;
int activeMouseButton = -1;
bool isDraggingFlask = false;
Vec3 dragPlaneOffset(0.0f, 0.0f, 0.0f);
Vec3 dragPlaneNormal(0.0f, 0.0f, 1.0f);
Vec3 dragPlanePoint(0.0f, 0.0f, 0.0f);

// Real-time FPS metric calculation (R7 Requirement)
int frameCount = 0;
float fpsAccumulator = 0.0f;
float currentFPS = 60.0f;
std::chrono::high_resolution_clock::time_point lastFrameTime;

void updateWindowTitle() {
    int fillPercent = (int)(scene.slosh.fillLevel * 100.0f);
    std::string title = "Alchemist's Potion Lab | FPS: " + std::to_string((int)currentFPS)
        + " | " + scene.colorName
        + " | Fill: " + std::to_string(fillPercent) + "%";
    glutSetWindowTitle(title.c_str());
}

void printHelp() {
    std::cout << "\n=======================================================\n";
    std::cout << "         ALCHEMIST'S POTION LAB - FLASK DEMO           \n";
    std::cout << "=======================================================\n";
    std::cout << " [Left Mouse Button (Drag)]  : Grab & drag flask across the screen\n";
    std::cout << "                               (Watch the dynamic sloshing fluid!)\n";
    std::cout << " [Right Mouse Button (Drag)] : Orbit 3D camera around the flask\n";
    std::cout << " [Mouse Scroll Wheel]        : Zoom camera in / out\n";
    std::cout << " [Space]                     : Agitate / swirl the potion\n";
    std::cout << " [1] - [6]                   : Switch potion color:\n";
    std::cout << "                               1: Emerald Green (Elixir of Life)\n";
    std::cout << "                               2: Ruby Crimson (Dragon's Breath)\n";
    std::cout << "                               3: Sapphire Blue (Mana Essence)\n";
    std::cout << "                               4: Amethyst Purple (Void Draught)\n";
    std::cout << "                               5: Auric Gold (Philosopher's Tincture)\n";
    std::cout << "                               6: Frost Cyan (Glacial Infusion)\n";
    std::cout << " [C]                         : Cycle potion colors smoothly\n";
    std::cout << " [+] / [-]                   : Increase / decrease liquid fill level\n";
    std::cout << " [R]                         : Reset flask position and settle fluid\n";
    std::cout << " [V]                         : Reset camera view\n";
    std::cout << " [H]                         : Print this controls help\n";
    std::cout << " [ESC]                       : Exit application\n";
    std::cout << "=======================================================\n\n";
}

void display() {
    glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float)windowWidth / (float)(windowHeight > 0 ? windowHeight : 1);
    Mat4 proj = Mat4::perspective(camera.fov, aspect, 0.1f, 50.0f);
    Mat4 view = camera.getViewMatrix();
    Vec3 camPos = camera.getPosition();

    scene.render(view, proj, camPos);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h > 0 ? h : 1;
    glViewport(0, 0, windowWidth, windowHeight);
}

void timer(int value) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - lastFrameTime;
    lastFrameTime = now;

    float dt = elapsed.count();
    if (dt > 0.05f) dt = 0.05f; // Clamp delta time

    // Frame rate counting
    frameCount++;
    fpsAccumulator += dt;
    if (fpsAccumulator >= 0.5f) {
        currentFPS = (float)frameCount / fpsAccumulator;
        frameCount = 0;
        fpsAccumulator = 0.0f;
        updateWindowTitle();
    }

    // Advance physical slosh simulation
    scene.update(dt);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // Target ~60 FPS
}

void mouseButton(int button, int state, int x, int y) {
    lastMouseX = x;
    lastMouseY = y;
    activeMouseButton = button;

    float aspect = (float)windowWidth / (float)(windowHeight > 0 ? windowHeight : 1);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            Vec3 rayOrigin, rayDir;
            camera.screenToRay(x, y, windowWidth, windowHeight, aspect, rayOrigin, rayDir);

            // Drag plane perpendicular to camera view direction passing through flask center
            dragPlaneNormal = (camera.getPosition() - camera.target).normalized();
            dragPlanePoint = scene.flaskPos + Vec3(0.0f, 0.22f, 0.0f);

            float denom = Vec3::dot(rayDir, dragPlaneNormal);
            if (std::fabs(denom) > 1e-4f) {
                float t = Vec3::dot(dragPlanePoint - rayOrigin, dragPlaneNormal) / denom;
                if (t > 0.0f) {
                    Vec3 hitPoint = rayOrigin + rayDir * t;
                    isDraggingFlask = true;
                    dragPlaneOffset = scene.flaskPos - (hitPoint - Vec3(0.0f, 0.22f, 0.0f));
                    scene.onFlaskDragStart();
                }
            }
        } else if (state == GLUT_UP) {
            if (isDraggingFlask) {
                isDraggingFlask = false;
                scene.onFlaskDragEnd();
            }
        }
    }
    // Mouse wheel zoom
    else if (button == 3) {
        camera.zoom(-0.2f);
    } else if (button == 4) {
        camera.zoom(0.2f);
    }
}

void mouseMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;

    float aspect = (float)windowWidth / (float)(windowHeight > 0 ? windowHeight : 1);

    if (isDraggingFlask && activeMouseButton == GLUT_LEFT_BUTTON) {
        Vec3 rayOrigin, rayDir;
        camera.screenToRay(x, y, windowWidth, windowHeight, aspect, rayOrigin, rayDir);

        float denom = Vec3::dot(rayDir, dragPlaneNormal);
        if (std::fabs(denom) > 1e-4f) {
            float t = Vec3::dot(dragPlanePoint - rayOrigin, dragPlaneNormal) / denom;
            if (t > 0.0f) {
                Vec3 hitPoint = rayOrigin + rayDir * t;
                Vec3 newPos = (hitPoint - Vec3(0.0f, 0.22f, 0.0f)) + dragPlaneOffset;

                // Clamp to screen view boundaries
                newPos.x = std::max(-1.8f, std::min(1.8f, newPos.x));
                newPos.y = std::max(-1.0f, std::min(1.0f, newPos.y));
                newPos.z = std::max(-1.5f, std::min(1.5f, newPos.z));

                scene.onFlaskDrag(newPos);
            }
        }
    } else if (activeMouseButton == GLUT_RIGHT_BUTTON) {
        // Smooth camera orbit
        float rotSpeed = 0.006f;
        camera.rotate(-dx * rotSpeed, dy * rotSpeed);
    }
}

void mousePassiveMotion(int x, int y) {
    lastMouseX = x;
    lastMouseY = y;
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC
            glutLeaveMainLoop();
            break;
        case ' ': // Stir / agitate fluid
            scene.stirPotion();
            std::cout << "[Alchemy] Stirred flask with swirling fluid impulse!\n";
            break;
        case '1':
            scene.setColorPreset(1);
            std::cout << "[Alchemy] Transmuted potion to Emerald Green (Elixir of Life)!\n";
            updateWindowTitle();
            break;
        case '2':
            scene.setColorPreset(2);
            std::cout << "[Alchemy] Transmuted potion to Ruby Crimson (Dragon's Breath)!\n";
            updateWindowTitle();
            break;
        case '3':
            scene.setColorPreset(3);
            std::cout << "[Alchemy] Transmuted potion to Sapphire Blue (Mana Essence)!\n";
            updateWindowTitle();
            break;
        case '4':
            scene.setColorPreset(4);
            std::cout << "[Alchemy] Transmuted potion to Amethyst Purple (Void Draught)!\n";
            updateWindowTitle();
            break;
        case '5':
            scene.setColorPreset(5);
            std::cout << "[Alchemy] Transmuted potion to Auric Gold (Philosopher's Tincture)!\n";
            updateWindowTitle();
            break;
        case '6':
            scene.setColorPreset(6);
            std::cout << "[Alchemy] Transmuted potion to Frost Cyan (Glacial Infusion)!\n";
            updateWindowTitle();
            break;
        case 'c':
        case 'C':
            scene.cycleColor();
            std::cout << "[Alchemy] Cycled potion color to: " << scene.colorName << "\n";
            updateWindowTitle();
            break;
        case '+':
        case '=':
            scene.adjustLiquidLevel(0.05f);
            std::cout << "[Alchemy] Increased liquid level to " << (int)(scene.slosh.fillLevel * 100.0f) << "%\n";
            updateWindowTitle();
            break;
        case '-':
        case '_':
            scene.adjustLiquidLevel(-0.05f);
            std::cout << "[Alchemy] Decreased liquid level to " << (int)(scene.slosh.fillLevel * 100.0f) << "%\n";
            updateWindowTitle();
            break;
        case 'r':
        case 'R':
            scene.resetPotion();
            std::cout << "[Alchemy] Reset flask to center.\n";
            updateWindowTitle();
            break;
        case 'v':
        case 'V':
            camera.reset();
            std::cout << "[Camera] Reset view.\n";
            break;
        case 'h':
        case 'H':
            printHelp();
            break;
        default:
            break;
    }
}

int main(int argc, char** argv) {
    std::cout << "========================================================\n";
    std::cout << " Starting Alchemist's Potion Lab (Dynamic Slosh Demo)   \n";
    std::cout << " OpenGL 3.3 Core Profile | C++14                        \n";
    std::cout << "========================================================\n";

    // 1. Initialize FreeGLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);

    // OpenGL 3.3 Core Profile configuration
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    int window = glutCreateWindow("Alchemist's Potion Lab");
    if (window < 1) {
        std::cerr << "[ERROR] Failed to create FreeGLUT window!\n";
        return -1;
    }

    // 2. Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "[ERROR] GLEW initialization error: " << glewGetErrorString(err) << "\n";
        return -1;
    }

    // Clear any benign GLEW initialization flag error
    glGetError();

    std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version  : " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // 3. Initialize Scene & Shaders
    scene.init();

    // Setup initial camera framing
    camera.target = Vec3(0.0f, 0.22f, 0.0f);
    camera.distance = 2.2f;
    camera.pitch = 0.32f;
    camera.yaw = 0.0f;

    // 4. Register FreeGLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutKeyboardFunc(keyboard);

    lastFrameTime = std::chrono::high_resolution_clock::now();
    glutTimerFunc(16, timer, 0);

    printHelp();
    updateWindowTitle();

    // 5. Enter main loop
    glutMainLoop();

    return 0;
}
