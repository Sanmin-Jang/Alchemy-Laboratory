#include "SloshPhysics.h"
#include <algorithm>
#include <cmath>

SloshPhysics::SloshPhysics()
    : currentPos(0.0f, 0.0f, 0.0f),
      prevPos(0.0f, 0.0f, 0.0f),
      velocity(0.0f, 0.0f, 0.0f),
      prevVelocity(0.0f, 0.0f, 0.0f),
      acceleration(0.0f, 0.0f, 0.0f),
      tilt(0.0f, 0.0f),
      tiltVelocity(0.0f, 0.0f),
      wavePhase(0.0f),
      waveAmplitude(0.0f),
      naturalFreq(8.5f),      // ~1.35 Hz natural slosh frequency
      damping(2.8f),          // viscous decay
      inertiaScale(0.18f),    // acceleration response
      maxTiltAngle(0.48f),    // ~27.5 degrees max surface tilt
      fillLevel(0.65f),       // default 65% full
      liquidColor(0.15f, 0.95f, 0.35f, 0.88f) // glowing emerald potion
{}

void SloshPhysics::update(const Vec3& newFlaskPos, float dt) {
    if (dt <= 0.0f) return;
    if (dt > 0.05f) dt = 0.05f; // prevent simulation explosion on large frame drops

    // 1. Calculate flask kinematics (position -> velocity -> acceleration)
    currentPos = newFlaskPos;
    Vec3 rawVelocity = (currentPos - prevPos) / dt;
    // Smooth velocity slightly to prevent jerky mouse quantization
    velocity = Vec3::lerp(velocity, rawVelocity, 0.75f);

    Vec3 rawAcceleration = (velocity - prevVelocity) / dt;
    acceleration = Vec3::lerp(acceleration, rawAcceleration, 0.65f);

    prevPos = currentPos;
    prevVelocity = velocity;

    // 2. Drive the 2D liquid tilt oscillator using Damped Harmonic Motion:
    // d^2(theta)/dt^2 = -omega^2 * theta - 2 * damping * d(theta)/dt - inertiaScale * accel
    // Notice: linear acceleration in +X tilts the liquid surface backwards (-X)
    float drivingForceX = -acceleration.x * inertiaScale;
    float drivingForceZ = -acceleration.z * inertiaScale;

    // Semi-implicit Euler integration
    float omegaSq = naturalFreq * naturalFreq;

    float d2ThetaX = drivingForceX - omegaSq * tilt.x - damping * tiltVelocity.x;
    float d2ThetaZ = drivingForceZ - omegaSq * tilt.y - damping * tiltVelocity.y;

    tiltVelocity.x += d2ThetaX * dt;
    tiltVelocity.y += d2ThetaZ * dt;

    tilt.x += tiltVelocity.x * dt;
    tilt.y += tiltVelocity.y * dt;

    // 3. Clamp tilt to physical flask boundaries
    float tiltLen = tilt.length();
    if (tiltLen > maxTiltAngle) {
        tilt = tilt * (maxTiltAngle / tiltLen);
        // Bounce off glass walls with restitution
        tiltVelocity = tiltVelocity * -0.3f;
    }

    // 4. Update surface ripple wave phase and amplitude
    float motionEnergy = velocity.length() * 0.4f + tiltVelocity.length() * 0.15f;
    float targetAmp = std::min(0.08f, motionEnergy * 0.12f);
    waveAmplitude += (targetAmp - waveAmplitude) * (dt * 6.0f);
    wavePhase += dt * (12.0f + motionEnergy * 8.0f);
    if (wavePhase > 2.0f * (float)M_PI) {
        wavePhase = std::fmod(wavePhase, 2.0f * (float)M_PI);
    }
}

void SloshPhysics::addImpulse(float impX, float impZ) {
    tiltVelocity.x += impX;
    tiltVelocity.y += impZ;
}

void SloshPhysics::stir(float strength) {
    // Add swirling angular impulse
    tiltVelocity.x += 3.5f * strength;
    tiltVelocity.y -= 2.8f * strength;
    waveAmplitude = 0.08f * strength;
}

Vec3 SloshPhysics::getSurfaceNormal() const {
    // Normal vector of plane tilted by (tilt.x, tilt.y)
    // Tilted plane equation: -tilt.x * X + Y - tilt.z * Z = const
    Vec3 n(-tilt.x, 1.0f, -tilt.y);
    return n.normalized();
}

Mat4 SloshPhysics::getSurfaceTiltMatrix(float liquidCenterY) const {
    // Translate to liquid center, apply tilt rotations, translate back
    Mat4 toOrigin = Mat4::translate(0.0f, -liquidCenterY, 0.0f);
    Mat4 rotX = Mat4::rotateX(tilt.y); // tilt along Z axis
    Mat4 rotZ = Mat4::rotateZ(-tilt.x); // tilt along X axis
    Mat4 fromOrigin = Mat4::translate(0.0f, liquidCenterY, 0.0f);

    return fromOrigin * (rotZ * (rotX * toOrigin));
}
