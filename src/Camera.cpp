#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera()
    : mode(CAMERA_MENU),
      target(Vec3(0.0f, 0.25f, -0.05f)),
      distance(2.25f),
      orbitYaw(0.25f),
      orbitPitch(0.30f),
      minDistance(0.95f),
      maxDistance(5.5f),
      fpsPos(Vec3(0.0f, 0.72f, 1.95f)),
      fpsYaw(-(float)M_PI * 0.5f),
      fpsPitch(-0.22f),
      moveSpeed(2.4f),
      fov(48.0f * (float)M_PI / 180.0f),
      shakeOffset(0.0f, 0.0f, 0.0f) {}

Vec3 Camera::getForward() const {
    if (mode == CAMERA_FPS) {
        float cp = std::cos(fpsPitch);
        return Vec3(
            cp * std::cos(fpsYaw),
            std::sin(fpsPitch),
            cp * std::sin(fpsYaw)
        ).normalized();
    }
    return (target - getPosition()).normalized();
}

Vec3 Camera::getRight() const {
    return Vec3::cross(getForward(), Vec3(0.0f, 1.0f, 0.0f)).normalized();
}

Vec3 Camera::getPosition() const {
    if (mode == CAMERA_FPS) {
        return fpsPos + shakeOffset;
    }
    float cp = std::cos(orbitPitch);
    float sp = std::sin(orbitPitch);
    float cy = std::cos(orbitYaw);
    float sy = std::sin(orbitYaw);
    return Vec3(
        target.x + distance * cp * sy,
        target.y + distance * sp,
        target.z + distance * cp * cy
    ) + shakeOffset;
}

Mat4 Camera::getViewMatrix() const {
    Vec3 eye = getPosition();
    if (mode == CAMERA_FPS) {
        Vec3 fwd = getForward();
        return Mat4::lookAt(eye, eye + fwd, Vec3(0.0f, 1.0f, 0.0f));
    }
    return Mat4::lookAt(eye, target + shakeOffset * 0.35f, Vec3(0.0f, 1.0f, 0.0f));
}

void Camera::setMode(CameraMode newMode) {
    mode = newMode;
}

void Camera::togglePlayMode() {
    if (mode == CAMERA_FPS) {
        mode = CAMERA_ORBIT;
        target = Vec3(0.0f, 0.25f, -0.05f);
        distance = 2.15f;
        orbitYaw = 0.0f;
        orbitPitch = 0.30f;
    } else {
        mode = CAMERA_FPS;
        fpsPos = Vec3(0.0f, 0.72f, 1.95f);
        fpsYaw = -(float)M_PI * 0.5f;
        fpsPitch = -0.22f;
    }
}

void Camera::updateMenuOrbit(float dt) {
    orbitYaw += dt * 0.22f;
    orbitPitch = 0.28f + 0.05f * std::sin(orbitYaw * 1.5f);
    distance = 2.55f;
    target = Vec3(0.0f, 0.26f, -0.05f);
}

void Camera::moveFPS(float forwardInput, float rightInput, float dt) {
    if (mode != CAMERA_FPS) return;

    // Horizontal forward and right vectors on XZ plane for natural walking
    Vec3 fwdXZ(std::cos(fpsYaw), 0.0f, std::sin(fpsYaw));
    fwdXZ = fwdXZ.normalized();
    Vec3 rightXZ = Vec3::cross(fwdXZ, Vec3(0.0f, 1.0f, 0.0f)).normalized();

    Vec3 moveDir = fwdXZ * forwardInput + rightXZ * rightInput;
    if (moveDir.lengthSq() > 1e-6f) {
        moveDir = moveDir.normalized();
    }

    Vec3 nextPos = fpsPos + moveDir * (moveSpeed * dt);

    // Clamp inside stone dungeon room bounds
    nextPos.x = std::max(-3.15f, std::min(3.15f, nextPos.x));
    nextPos.z = std::max(-1.65f, std::min(3.25f, nextPos.z));

    // Simple AABB collision against oak workbench (table center (0, -0.15), half-extents + player radius)
    const float tMinX = -1.68f, tMaxX = 1.68f;
    const float tMinZ = -1.18f, tMaxZ = 0.88f;

    if (nextPos.x > tMinX && nextPos.x < tMaxX && nextPos.z > tMinZ && nextPos.z < tMaxZ) {
        // Resolve along axis of smallest penetration so player slides smoothly along table edges
        float dxLeft  = std::fabs(nextPos.x - tMinX);
        float dxRight = std::fabs(tMaxX - nextPos.x);
        float dzBack  = std::fabs(nextPos.z - tMinZ);
        float dzFront = std::fabs(tMaxZ - nextPos.z);
        float minPen = std::min(std::min(dxLeft, dxRight), std::min(dzBack, dzFront));
        if (minPen == dzFront)      nextPos.z = tMaxZ;
        else if (minPen == dzBack)  nextPos.z = tMinZ;
        else if (minPen == dxLeft)  nextPos.x = tMinX;
        else                        nextPos.x = tMaxX;
    }

    fpsPos = nextPos;
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    if (mode == CAMERA_FPS) {
        fpsYaw -= deltaYaw;
        fpsPitch -= deltaPitch;
        float limit = 75.0f * (float)M_PI / 180.0f;
        fpsPitch = std::max(-limit, std::min(limit, fpsPitch));
    } else if (mode == CAMERA_ORBIT) {
        orbitYaw += deltaYaw;
        orbitPitch += deltaPitch;
        float maxP = 80.0f * (float)M_PI / 180.0f;
        float minP = 6.0f * (float)M_PI / 180.0f;
        orbitPitch = std::max(minP, std::min(maxP, orbitPitch));
    }
}

void Camera::zoom(float deltaDistance) {
    if (mode == CAMERA_ORBIT) {
        distance = std::max(minDistance, std::min(maxDistance, distance + deltaDistance));
    } else if (mode == CAMERA_FPS) {
        // Subtle zoom via vertical eye height / crouch-inspect adjustment
        fpsPos.y = std::max(0.35f, std::min(1.25f, fpsPos.y - deltaDistance * 0.25f));
    }
}

void Camera::reset() {
    target = Vec3(0.0f, 0.25f, -0.05f);
    distance = 2.15f;
    orbitYaw = 0.0f;
    orbitPitch = 0.30f;
    fpsPos = Vec3(0.0f, 0.72f, 1.95f);
    fpsYaw = -(float)M_PI * 0.5f;
    fpsPitch = -0.22f;
}

void Camera::screenToRay(int mouseX, int mouseY, int screenWidth, int screenHeight,
                         float aspect, Vec3& outRayOrigin, Vec3& outRayDir) const {
    float ndcX = (2.0f * (float)mouseX) / (float)screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)mouseY) / (float)screenHeight;

    Mat4 proj = Mat4::perspective(fov, aspect, 0.1f, 50.0f);
    Mat4 view = getViewMatrix();
    Mat4 invVP = (proj * view).inverse();

    Vec4 nearWorld = invVP * Vec4(ndcX, ndcY, -1.0f, 1.0f);
    Vec4 farWorld  = invVP * Vec4(ndcX, ndcY,  1.0f, 1.0f);

    if (std::fabs(nearWorld.w) > 1e-6f) nearWorld = nearWorld * (1.0f / nearWorld.w);
    if (std::fabs(farWorld.w) > 1e-6f)  farWorld  = farWorld  * (1.0f / farWorld.w);

    outRayOrigin = nearWorld.xyz();
    outRayDir = (farWorld.xyz() - nearWorld.xyz()).normalized();
}
