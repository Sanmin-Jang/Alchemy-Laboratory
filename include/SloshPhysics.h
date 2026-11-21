#ifndef SLOSH_PHYSICS_H
#define SLOSH_PHYSICS_H

#include "math3d.h"

class SloshPhysics {
public:
    // Flask kinematics
    Vec3 currentPos;
    Vec3 prevPos;
    Vec3 velocity;
    Vec3 prevVelocity;
    Vec3 acceleration;

    // Liquid surface oscillator state
    Vec2 tilt;         // (theta_x, theta_z) in radians
    Vec2 tiltVelocity; // (dtheta_x/dt, dtheta_z/dt)
    float wavePhase;
    float waveAmplitude;

    // Physical parameters
    float naturalFreq;  // omega_0: natural sloshing oscillation frequency
    float damping;      // zeta: damping factor (viscosity)
    float inertiaScale; // coupling from flask acceleration to tilt
    float maxTiltAngle; // limit tilt so it doesn't escape the flask neck

    // Liquid properties
    float fillLevel;     // 0.0 (empty) to 1.0 (full)
    float boilIntensity; // 0.0 (calm) to 1.0 (boiling reaction)
    Vec4 liquidColor;    // RGBA color

    SloshPhysics();

    void update(const Vec3& newFlaskPos, float dt);
    void addImpulse(float impX, float impZ);
    void stir(float strength = 1.0f);

    // Returns the normal vector of the tilted liquid surface
    Vec3 getSurfaceNormal() const;

    // Returns a transformation matrix that tilts around the liquid center of mass
    Mat4 getSurfaceTiltMatrix(float liquidCenterY) const;
};

#endif // SLOSH_PHYSICS_H