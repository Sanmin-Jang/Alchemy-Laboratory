#include "Camera.h"
#include <algorithm>

Camera::Camera()
    : target(Vec3(0.0f, 0.22f, 0.0f)),
      distance(2.2f),
      yaw(0.0f),
      pitch(0.32f),
      fov(45.0f * (float)M_PI / 180.0f),
      minDistance(0.8f),
      maxDistance(8.0f) {}

Camera::Camera(const Vec3& targetPos, float dist, float y, float p)
    : target(targetPos),
      distance(dist),
      yaw(y),
      pitch(p),
      fov(45.0f * (float)M_PI / 180.0f),
      minDistance(0.8f),
      maxDistance(8.0f) {}

Vec3 Camera::getPosition() const {
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    return Vec3(
        target.x + distance * cp * sy,
        target.y + distance * sp,
        target.z + distance * cp * cy
    );
}

Mat4 Camera::getViewMatrix() const {
    Vec3 eye = getPosition();
    return Mat4::lookAt(eye, target, Vec3(0.0f, 1.0f, 0.0f));
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch += deltaPitch;

    // Clamp pitch to avoid gimbal lock / flipping upside down
    float maxP = 85.0f * (float)M_PI / 180.0f;
    float minP = 5.0f * (float)M_PI / 180.0f;
    pitch = std::max(minP, std::min(maxP, pitch));
}

void Camera::zoom(float deltaDistance) {
    distance += deltaDistance;
    distance = std::max(minDistance, std::min(maxDistance, distance));
}

void Camera::pan(float deltaX, float deltaY) {
    Vec3 forward = (target - getPosition()).normalized();
    Vec3 right = Vec3::cross(forward, Vec3(0.0f, 1.0f, 0.0f)).normalized();
    Vec3 up = Vec3::cross(right, forward).normalized();

    target += right * deltaX + up * deltaY;
}

void Camera::reset() {
    target = Vec3(0.0f, 0.22f, 0.0f);
    distance = 2.2f;
    yaw = 0.0f;
    pitch = 0.32f;
}

void Camera::screenToRay(int mouseX, int mouseY, int screenWidth, int screenHeight,
                         float aspect, Vec3& outRayOrigin, Vec3& outRayDir) const {
    float ndcX = (2.0f * (float)mouseX) / (float)screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)mouseY) / (float)screenHeight;

    Mat4 proj = Mat4::perspective(fov, aspect, 0.1f, 100.0f);
    Mat4 view = getViewMatrix();
    Mat4 invVP = (proj * view).inverse();

    Vec4 nearPointNdc(ndcX, ndcY, -1.0f, 1.0f);
    Vec4 farPointNdc(ndcX, ndcY, 1.0f, 1.0f);

    Vec4 nearWorld = invVP * nearPointNdc;
    Vec4 farWorld = invVP * farPointNdc;

    if (std::fabs(nearWorld.w) > 1e-6f) nearWorld = nearWorld * (1.0f / nearWorld.w);
    if (std::fabs(farWorld.w) > 1e-6f) farWorld = farWorld * (1.0f / farWorld.w);

    outRayOrigin = nearWorld.xyz();
    outRayDir = (farWorld.xyz() - nearWorld.xyz()).normalized();
}
