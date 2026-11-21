#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cctype>

#include "math3d.h"
#include "Camera.h"
#include "Scene.h"
#include "UIRenderer.h"

// Window dimensions
int windowWidth = 1280;
int windowHeight = 720;

// Core application modules
Camera camera;
Scene scene;
UIRenderer ui;

// Game state & input tracking
GameState gameState = STATE_MAIN_MENU;
GameState recipesReturnState = STATE_MAIN_MENU;
bool keyDown[256] = { false };
bool freeLookEnabled = false;

int lastMouseX = 640;
int lastMouseY = 360;
int curMouseX = 640;
int curMouseY = 360;
int activeMouseButton = -1;
bool isDraggingFlask = false;
Vec3 dragPlaneOffset(0.0f, 0.0f, 0.0f);
Vec3 dragPlaneNormal(0.0f, 0.0f, 1.0f);
Vec3 dragPlanePoint(0.0f, 0.0f, 0.0f);

// Real-time FPS metrics (R7)
int frameCount = 0;
float fpsAccumulator = 0.0f;
float currentFPS = 60.0f;
std::chrono::high_resolution_clock::time_point lastFrameTime;

bool testMode = false;
int testFrameCounter = 0;

void updateWindowTitle() {
    std::string title = "Alchemist's Potion Lab | FPS: " + std::to_string((int)(currentFPS + 0.5f))
                      + " | " + scene.potions.currentPotionTitle
                      + " | Recipes: " + std::to_string(scene.potions.getDiscoveredCount()) + "/3";
    glutSetWindowTitle(title.c_str());
}

void handleMenuAction(int actionId) {
    switch (actionId) {
        case 1: // Enter Laboratory
            gameState = STATE_PLAYING;
            camera.setMode(CAMERA_FPS);
            camera.reset();
            break;
        case 2: // Recipes & Controls from Main Menu
            recipesReturnState = STATE_MAIN_MENU;
            gameState = STATE_RECIPES;
            break;
        case 3: // Quit
        case 13:
            glutLeaveMainLoop();
            break;
        case 10: // Resume from Pause
            gameState = STATE_PLAYING;
            break;
        case 11: // Recipes & Controls from Pause
            recipesReturnState = STATE_PAUSED;
            gameState = STATE_RECIPES;
            break;
        case 12: // Return to Main Menu
            gameState = STATE_MAIN_MENU;
            freeLookEnabled = false;
            camera.setMode(CAMERA_MENU);
            break;
        case 20: // Back from Recipes
            gameState = recipesReturnState;
            break;
        default:
            break;
    }
}

void display() {
    Vec3 bg = scene.getBackgroundFogColor();
    glClearColor(bg.x, bg.y, bg.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float)windowWidth / (float)(windowHeight > 0 ? windowHeight : 1);
    camera.shakeOffset = scene.getCameraShakeOffset();

    Mat4 proj = Mat4::perspective(camera.fov, aspect, 0.1f, 50.0f);
    Mat4 view = camera.getViewMatrix();
    Vec3 camPos = camera.getPosition();

    // 1. Render 3D Alchemist's Laboratory Scene
    scene.render(view, proj, camPos);

    // 2. Render Pure OpenGL 3.3 Core Profile 2D Menu, HUD & FPS Counter
    HUDState hud;
    hud.fps = currentFPS;
    hud.camMode = camera.mode;
    hud.candlesLit = scene.candlesLit;
    hud.fillLevel = scene.slosh.fillLevel;
    hud.particleCount = scene.particles.getActiveCount();
    hud.freeLook = freeLookEnabled;
    hud.potions = &scene.potions;
    ui.render(gameState, windowWidth, windowHeight, curMouseX, curMouseY, hud);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = std::max(1, w);
    windowHeight = std::max(1, h);
    glViewport(0, 0, windowWidth, windowHeight);
}

void idle() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - lastFrameTime;
    lastFrameTime = now;

    float dt = std::max(0.0001f, std::min(0.05f, elapsed.count()));

    // Frame-rate independent FPS counter (R7)
    frameCount++;
    fpsAccumulator += dt;
    if (fpsAccumulator >= 0.35f) {
        currentFPS = (float)frameCount / fpsAccumulator;
        frameCount = 0;
        fpsAccumulator = 0.0f;
        updateWindowTitle();
    }

    if (gameState == STATE_MAIN_MENU) {
        camera.updateMenuOrbit(dt);
    } else if (gameState == STATE_PLAYING && camera.mode == CAMERA_FPS) {
        float fwd = (keyDown['w'] ? 1.0f : 0.0f) - (keyDown['s'] ? 1.0f : 0.0f);
        float rgt = (keyDown['d'] ? 1.0f : 0.0f) - (keyDown['a'] ? 1.0f : 0.0f);
        if (fwd != 0.0f || rgt != 0.0f) {
            camera.moveFPS(fwd, rgt, dt);
        }
    }

    if (gameState != STATE_PAUSED) {
        scene.update(dt);
    }

    if (testMode) {
        testFrameCounter++;
        if (testFrameCounter >= 20) {
            std::cout << "[Test Mode] Rendered 20 frames cleanly at ~" << currentFPS << " FPS.\n";
            glutLeaveMainLoop();
            return;
        }
    }

    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    lastMouseX = curMouseX = x;
    lastMouseY = curMouseY = y;
    activeMouseButton = (state == GLUT_DOWN) ? button : -1;

    if (gameState != STATE_PLAYING) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            int action = ui.hitTestButton((float)x, (float)y);
            if (action >= 0) handleMenuAction(action);
        }
        return;
    }

    float aspect = (float)windowWidth / (float)(windowHeight > 0 ? windowHeight : 1);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            Vec3 rayOrigin, rayDir;
            camera.screenToRay(x, y, windowWidth, windowHeight, aspect, rayOrigin, rayDir);

            // Priority 1: Pick one of the 3 ingredient bottles on the workbench
            IngredientType pickedIng = scene.hitTestIngredient(rayOrigin, rayDir);
            if (pickedIng != ING_NONE) {
                scene.addIngredient(pickedIng);
                updateWindowTitle();
                return;
            }

            // Priority 2: Grab & drag the main Erlenmeyer flask to slosh liquid
            float hitDist = 0.0f;
            if (scene.hitTestFlask(rayOrigin, rayDir, hitDist)) {
                dragPlaneNormal = (camera.getPosition() - (scene.flaskPos + Vec3(0.0f, 0.22f, 0.0f))).normalized();
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
            }
        } else if (state == GLUT_UP && isDraggingFlask) {
            isDraggingFlask = false;
            scene.onFlaskDragEnd();
        }
    } else if (button == 3 && state == GLUT_DOWN) {
        camera.zoom(-0.18f);
    } else if (button == 4 && state == GLUT_DOWN) {
        camera.zoom(0.18f);
    }
}

void mouseMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = curMouseX = x;
    lastMouseY = curMouseY = y;

    if (gameState != STATE_PLAYING) return;

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
                newPos.x = std::max(-1.15f, std::min(1.15f, newPos.x));
                newPos.y = std::max(0.0f, std::min(0.95f, newPos.y));
                newPos.z = std::max(-0.65f, std::min(0.55f, newPos.z));
                scene.onFlaskDrag(newPos);
            }
        }
    } else if (activeMouseButton == GLUT_RIGHT_BUTTON) {
        camera.rotate(-dx * 0.0055f, dy * 0.0055f);
    }
}

void mousePassiveMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = curMouseX = x;
    lastMouseY = curMouseY = y;

    if (gameState == STATE_PLAYING && freeLookEnabled && camera.mode == CAMERA_FPS) {
        camera.rotate(-dx * 0.0045f, dy * 0.0045f);
    }
}

void keyboardDown(unsigned char key, int x, int y) {
    (void)x; (void)y;
    unsigned char lower = (unsigned char)std::tolower(key);
    keyDown[lower] = true;

    if (gameState == STATE_MAIN_MENU) {
        if (key == 13 || key == ' ' || key == '1') handleMenuAction(1);
        else if (key == '2' || lower == 'h')       handleMenuAction(2);
        else if (key == '3' || key == 27)          handleMenuAction(3);
        return;
    }

    if (gameState == STATE_RECIPES) {
        if (key == 27 || lower == 'h' || key == 13) handleMenuAction(20);
        return;
    }

    if (gameState == STATE_PAUSED) {
        if (key == 27) handleMenuAction(10);
        return;
    }

    switch (lower) {
        case 27: // ESC -> Pause Menu
            freeLookEnabled = false;
            gameState = STATE_PAUSED;
            break;
        case '1':
            scene.addIngredient(ING_EMBER);
            updateWindowTitle();
            break;
        case '2':
            scene.addIngredient(ING_FROST);
            updateWindowTitle();
            break;
        case '3':
            scene.addIngredient(ING_VENOM);
            updateWindowTitle();
            break;
        case 'e':
        case 13:
            scene.brewPotion();
            updateWindowTitle();
            break;
        case ' ':
            scene.stirPotion();
            break;
        case 'l':
            scene.toggleCandles();
            break;
        case '+':
        case '=':
            scene.adjustLiquidLevel(0.06f);
            break;
        case '-':
        case '_':
            scene.adjustLiquidLevel(-0.06f);
            break;
        case 'r':
            scene.resetPotion();
            updateWindowTitle();
            break;
        case 'v':
            camera.togglePlayMode();
            break;
        case 'f':
            freeLookEnabled = !freeLookEnabled;
            break;
        case 'h':
            recipesReturnState = STATE_PLAYING;
            gameState = STATE_RECIPES;
            break;
        default:
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    (void)x; (void)y;
    keyDown[(unsigned char)std::tolower(key)] = false;
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") testMode = true;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(80, 60);

    // Enforce OpenGL 3.3 Core Profile (R1)
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    if (glutCreateWindow("Alchemist's Potion Lab") < 1) {
        std::cerr << "[ERROR] Failed to create FreeGLUT window.\n";
        return -1;
    }

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "[ERROR] GLEW init failed: " << glewGetErrorString(err) << "\n";
        return -1;
    }
    glGetError();

    std::cout << "Alchemist's Potion Lab | OpenGL " << glGetString(GL_VERSION)
              << " | Renderer: " << glGetString(GL_RENDERER) << "\n";

    scene.init();
    ui.init();

    glutIgnoreKeyRepeat(1);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    lastFrameTime = std::chrono::high_resolution_clock::now();
    updateWindowTitle();
    glutMainLoop();
    return 0;
}
