#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

enum CameraMode {
    CAMERA_FPS = 0,
    CAMERA_ORBIT = 1,
    CAMERA_MENU = 2
};

class Camera {
public:
    CameraMode mode;

    // Orbit camera parameters
    Vec3 target;
    float distance;
    float orbitYaw;
    float orbitPitch;
    float minDistance;
    float maxDistance;

    // FPS walking camera parameters
    Vec3 fpsPos;
    float fpsYaw;    // radians (-PI/2 faces -Z)
    float fpsPitch;  // radians
    float moveSpeed;

    // Shared projection & FX
    float fov;
    Vec3 shakeOffset;

    Camera();

    Mat4 getViewMatrix() const;
    Vec3 getPosition() const;
    Vec3 getForward() const;
    Vec3 getRight() const;

    void setMode(CameraMode newMode);
    void togglePlayMode();
    void updateMenuOrbit(float dt);

    // Frame-rate independent FPS walking with room & workbench collision
    void moveFPS(float forwardInput, float rightInput, float dt);

    // Mouse look / orbit rotation
    void rotate(float deltaYaw, float deltaPitch);
    void zoom(float deltaDistance);
    void reset();

    // Unproject screen coordinates (x, y) into a normalized world ray
    void screenToRay(int mouseX, int mouseY, int screenWidth, int screenHeight,
                     float aspect, Vec3& outRayOrigin, Vec3& outRayDir) const;
};

#endif // CAMERA_H
