#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <GL/glew.h>
#include <vector>
#include "math3d.h"
#include "Shader.h"
#include "Texture.h"

enum ParticleKind {
    PARTICLE_BUBBLE = 0,
    PARTICLE_STEAM,
    PARTICLE_EMBER,
    PARTICLE_SMOKE,
    PARTICLE_ARCANE_SPARK,
    PARTICLE_FROST_MIST,
    PARTICLE_AMBIENT_DUST
};

struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 colorStart;
    Vec4 colorEnd;
    float sizeStart;
    float sizeEnd;
    float age;
    float lifetime;
    float spiralAngle;
    ParticleKind kind;
    bool additive;
};

struct ParticleInstanceData {
    float x, y, z, size;
    float r, g, b, a;
};

class ParticleSystem {
public:
    static const size_t MAX_PARTICLES = 1200;

    ParticleSystem();
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    void init();
    void update(float dt, const Vec3& flaskPos);
    void render(const Mat4& view, const Mat4& proj);

    // Emitters for continuous alchemy effects and potion combinations
    void emitBubble(const Vec3& flaskPos, float liquidTopY, const Vec3& color);
    void emitSteam(const Vec3& mouthPos, const Vec3& color, int count = 1);
    void emitSplash(const Vec3& mouthPos, const Vec3& color, int count = 14);

    // 3 Magical Potion Combination Visual Effects
    void triggerExplosionBurst(const Vec3& origin);
    void emitArcaneSpiral(const Vec3& flaskPos, int count = 4);
    void emitFrostMist(const Vec3& flaskPos, int count = 5);
    void clearReactionParticles();

    size_t getActiveCount() const { return particles.size(); }

private:
    Shader shader;
    Texture particleTex;

    GLuint vao;
    GLuint quadVbo;
    GLuint quadEbo;
    GLuint instanceVbo;

    std::vector<Particle> particles;
    std::vector<ParticleInstanceData> alphaBatch;
    std::vector<ParticleInstanceData> additiveBatch;

    float dustTimer;
    unsigned int rngState;

    float randFloat();
    float randRange(float lo, float hi);
    void spawnAmbientDust();
    void spawnParticle(const Particle& p);
    void drawBatch(const std::vector<ParticleInstanceData>& batch);
};

#endif // PARTICLE_SYSTEM_H
