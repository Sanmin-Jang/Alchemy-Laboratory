#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

class Camera {
public:
    Vec3 target;
    float distance;
    float yaw;    // in radians
    float pitch;  // in radians
    float fov;    // in radians
    float minDistance;
    float maxDistance;

    Camera();
    Camera(const Vec3& targetPos, float dist = 4.0f, float y = 0.0f, float p = 0.5f);

    Mat4 getViewMatrix() const;
    Vec3 getPosition() const;

    void rotate(float deltaYaw, float deltaPitch);
    void zoom(float deltaDistance);
    void pan(float deltaX, float deltaY);
    void reset();

    // Convert screen coordinates (x, y) into a normalized world ray
    void screenToRay(int mouseX, int mouseY, int screenWidth, int screenHeight,
                     float aspect, Vec3& outRayOrigin, Vec3& outRayDir) const;
};

#endif // CAMERA_H