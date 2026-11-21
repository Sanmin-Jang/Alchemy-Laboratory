#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

ParticleSystem::ParticleSystem()
    : vao(0), quadVbo(0), quadEbo(0), instanceVbo(0), dustTimer(0.0f), rngState(0x9E3779B9u) {}

ParticleSystem::~ParticleSystem() {
    if (instanceVbo) glDeleteBuffers(1, &instanceVbo);
    if (quadEbo) glDeleteBuffers(1, &quadEbo);
    if (quadVbo) glDeleteBuffers(1, &quadVbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

float ParticleSystem::randFloat() {
    // Fast deterministic xorshift32 PRNG
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return (float)(rngState & 0x00FFFFFFu) / (float)0x01000000u;
}

float ParticleSystem::randRange(float lo, float hi) {
    return lo + (hi - lo) * randFloat();
}

void ParticleSystem::init() {
    shader.load("shaders/particle.vert", "shaders/particle.frag");
    particleTex = Texture::createParticleTexture(128);

    // Base quad geometry (pos.xyz + uv.xy)
    const float quadVerts[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };
    const unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &quadVbo);
    glGenBuffers(1, &quadEbo);
    glGenBuffers(1, &instanceVbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // location 0: quad position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // location 2: quad UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Per-instance buffer pre-allocated for MAX_PARTICLES
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(ParticleInstanceData), nullptr, GL_STREAM_DRAW);

    // location 3: vec4(worldPos.xyz, size)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)0);
    glVertexAttribDivisor(3, 1);

    // location 4: vec4(color.rgba)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)(4 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);

    particles.reserve(MAX_PARTICLES);
    alphaBatch.reserve(MAX_PARTICLES);
    additiveBatch.reserve(MAX_PARTICLES);

    // Seed initial floating laboratory dust motes
    for (int i = 0; i < 45; ++i) {
        spawnAmbientDust();
        if (!particles.empty()) {
            particles.back().age = randRange(0.0f, particles.back().lifetime * 0.8f);
        }
    }
}

void ParticleSystem::spawnParticle(const Particle& p) {
    if (particles.size() < MAX_PARTICLES) {
        particles.push_back(p);
    } else {
        // Overwrite oldest particle to keep ring performance O(1)
        particles[rngState % MAX_PARTICLES] = p;
    }
}

void ParticleSystem::spawnAmbientDust() {
    Particle p;
    p.position = Vec3(randRange(-2.6f, 2.6f), randRange(-0.4f, 2.2f), randRange(-1.8f, 2.2f));
    p.velocity = Vec3(randRange(-0.04f, 0.04f), randRange(0.01f, 0.04f), randRange(-0.04f, 0.04f));
    p.colorStart = Vec4(1.0f, 0.86f, 0.58f, 0.32f);
    p.colorEnd   = Vec4(0.85f, 0.78f, 0.65f, 0.0f);
    p.sizeStart  = randRange(0.014f, 0.028f);
    p.sizeEnd    = p.sizeStart * 0.8f;
    p.age = 0.0f;
    p.lifetime = randRange(4.5f, 8.5f);
    p.spiralAngle = randRange(0.0f, 6.28f);
    p.kind = PARTICLE_AMBIENT_DUST;
    p.additive = true;
    spawnParticle(p);
}

void ParticleSystem::emitBubble(const Vec3& flaskPos, float liquidTopY, const Vec3& color) {
    Particle p;
    float angle = randRange(0.0f, 6.28318f);
    float rad = randRange(0.01f, 0.11f);
    p.position = flaskPos + Vec3(std::cos(angle) * rad, randRange(0.04f, std::max(0.08f, liquidTopY * 0.8f)), std::sin(angle) * rad);
    p.velocity = Vec3(randRange(-0.02f, 0.02f), randRange(0.16f, 0.34f), randRange(-0.02f, 0.02f));
    p.colorStart = Vec4(color.x * 0.6f + 0.4f, color.y * 0.6f + 0.4f, color.z * 0.6f + 0.4f, 0.85f);
    p.colorEnd   = Vec4(1.0f, 1.0f, 1.0f, 0.0f);
    p.sizeStart  = randRange(0.018f, 0.036f);
    p.sizeEnd    = p.sizeStart * 1.35f;
    p.age = 0.0f;
    p.lifetime = randRange(0.65f, 1.25f);
    p.spiralAngle = angle;
    p.kind = PARTICLE_BUBBLE;
    p.additive = true;
    spawnParticle(p);
}

void ParticleSystem::emitSteam(const Vec3& mouthPos, const Vec3& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = mouthPos + Vec3(randRange(-0.03f, 0.03f), randRange(0.0f, 0.03f), randRange(-0.03f, 0.03f));
        p.velocity = Vec3(randRange(-0.08f, 0.08f), randRange(0.24f, 0.48f), randRange(-0.08f, 0.08f));
        Vec3 tint = Vec3::lerp(Vec3(0.85f, 0.88f, 0.92f), color, 0.38f);
        p.colorStart = Vec4(tint, 0.42f);
        p.colorEnd   = Vec4(tint, 0.0f);
        p.sizeStart  = randRange(0.06f, 0.11f);
        p.sizeEnd    = randRange(0.22f, 0.36f);
        p.age = 0.0f;
        p.lifetime = randRange(1.2f, 2.3f);
        p.spiralAngle = randRange(0.0f, 6.28f);
        p.kind = PARTICLE_STEAM;
        p.additive = false;
        spawnParticle(p);
    }
}

void ParticleSystem::emitSplash(const Vec3& mouthPos, const Vec3& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        float ang = randRange(0.0f, 6.28318f);
        float spd = randRange(0.15f, 0.55f);
        p.position = mouthPos;
        p.velocity = Vec3(std::cos(ang) * spd, randRange(0.35f, 0.85f), std::sin(ang) * spd);
        p.colorStart = Vec4(color * 1.15f, 0.95f);
        p.colorEnd   = Vec4(color, 0.0f);
        p.sizeStart  = randRange(0.025f, 0.05f);
        p.sizeEnd    = 0.008f;
        p.age = 0.0f;
        p.lifetime = randRange(0.45f, 0.85f);
        p.spiralAngle = 0.0f;
        p.kind = PARTICLE_EMBER;
        p.additive = true;
        spawnParticle(p);
    }
}

void ParticleSystem::triggerExplosionBurst(const Vec3& origin) {
    // 1. Bright fiery radial embers
    for (int i = 0; i < 140; ++i) {
        Particle p;
        float theta = randRange(0.0f, 6.28318f);
        float phi   = randRange(0.15f, 2.4f);
        float speed = randRange(0.9f, 3.2f);
        p.position = origin + Vec3(randRange(-0.04f, 0.04f), randRange(0.0f, 0.08f), randRange(-0.04f, 0.04f));
        p.velocity = Vec3(std::sin(phi) * std::cos(theta) * speed,
                          std::fabs(std::cos(phi)) * speed * 1.15f + 0.5f,
                          std::sin(phi) * std::sin(theta) * speed);
        p.colorStart = Vec4(1.0f, 0.92f, 0.35f, 1.0f);
        p.colorEnd   = Vec4(0.95f, 0.14f, 0.02f, 0.0f);
        p.sizeStart  = randRange(0.045f, 0.11f);
        p.sizeEnd    = 0.015f;
        p.age = 0.0f;
        p.lifetime = randRange(0.55f, 1.55f);
        p.spiralAngle = 0.0f;
        p.kind = PARTICLE_EMBER;
        p.additive = true;
        spawnParticle(p);
    }

    // 2. Billowing dark smoke & hot soot clouds
    for (int i = 0; i < 65; ++i) {
        Particle p;
        float theta = randRange(0.0f, 6.28318f);
        float speed = randRange(0.25f, 1.1f);
        p.position = origin + Vec3(randRange(-0.08f, 0.08f), randRange(0.02f, 0.15f), randRange(-0.08f, 0.08f));
        p.velocity = Vec3(std::cos(theta) * speed, randRange(0.45f, 1.35f), std::sin(theta) * speed);
        p.colorStart = Vec4(0.32f, 0.18f, 0.12f, 0.72f);
        p.colorEnd   = Vec4(0.08f, 0.08f, 0.10f, 0.0f);
        p.sizeStart  = randRange(0.14f, 0.25f);
        p.sizeEnd    = randRange(0.48f, 0.78f);
        p.age = 0.0f;
        p.lifetime = randRange(1.4f, 2.8f);
        p.spiralAngle = theta;
        p.kind = PARTICLE_SMOKE;
        p.additive = false;
        spawnParticle(p);
    }
}

void ParticleSystem::emitArcaneSpiral(const Vec3& flaskPos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        float angle = randRange(0.0f, 6.28318f);
        float rad = randRange(0.18f, 0.42f);
        p.position = flaskPos + Vec3(std::cos(angle) * rad, randRange(0.04f, 0.28f), std::sin(angle) * rad);
        p.velocity = Vec3(0.0f, randRange(0.35f, 0.75f), 0.0f);
        bool gold = (randFloat() > 0.45f);
        p.colorStart = gold ? Vec4(1.0f, 0.88f, 0.35f, 1.0f) : Vec4(0.86f, 0.32f, 1.0f, 1.0f);
        p.colorEnd   = gold ? Vec4(1.0f, 0.55f, 0.95f, 0.0f) : Vec4(0.35f, 0.80f, 1.0f, 0.0f);
        p.sizeStart  = randRange(0.032f, 0.068f);
        p.sizeEnd    = 0.01f;
        p.age = 0.0f;
        p.lifetime = randRange(1.3f, 2.2f);
        p.spiralAngle = angle;
        p.kind = PARTICLE_ARCANE_SPARK;
        p.additive = true;
        spawnParticle(p);
    }
}

void ParticleSystem::emitFrostMist(const Vec3& flaskPos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        float angle = randRange(0.0f, 6.28318f);
        float speed = randRange(0.18f, 0.52f);
        bool isCrystal = (randFloat() > 0.55f);
        p.position = flaskPos + Vec3(std::cos(angle) * 0.06f, 0.46f, std::sin(angle) * 0.06f);
        p.velocity = Vec3(std::cos(angle) * speed, randRange(0.08f, 0.28f), std::sin(angle) * speed);
        if (isCrystal) {
            p.colorStart = Vec4(0.75f, 0.96f, 1.0f, 0.95f);
            p.colorEnd   = Vec4(0.18f, 0.72f, 1.0f, 0.0f);
            p.sizeStart  = randRange(0.025f, 0.055f);
            p.sizeEnd    = 0.012f;
            p.additive   = true;
        } else {
            p.colorStart = Vec4(0.55f, 0.88f, 1.0f, 0.44f);
            p.colorEnd   = Vec4(0.35f, 0.75f, 0.98f, 0.0f);
            p.sizeStart  = randRange(0.12f, 0.22f);
            p.sizeEnd    = randRange(0.34f, 0.56f);
            p.additive   = false;
        }
        p.age = 0.0f;
        p.lifetime = randRange(1.6f, 2.8f);
        p.spiralAngle = angle;
        p.kind = PARTICLE_FROST_MIST;
        spawnParticle(p);
    }
}

void ParticleSystem::clearReactionParticles() {
    for (size_t i = 0; i < particles.size();) {
        if (particles[i].kind != PARTICLE_AMBIENT_DUST) {
            particles[i] = particles.back();
            particles.pop_back();
        } else {
            ++i;
        }
    }
}

void ParticleSystem::update(float dt, const Vec3& flaskPos) {
    dustTimer += dt;
    if (dustTimer >= 0.18f) {
        dustTimer = 0.0f;
        spawnAmbientDust();
    }

    for (size_t i = 0; i < particles.size();) {
        Particle& p = particles[i];
        p.age += dt;
        if (p.age >= p.lifetime) {
            particles[i] = particles.back();
            particles.pop_back();
            continue;
        }

        switch (p.kind) {
            case PARTICLE_EMBER:
                p.velocity.y -= 2.4f * dt; // Gravity on sparks
                p.velocity *= (1.0f - 0.8f * dt);
                p.position += p.velocity * dt;
                if (p.position.y < 0.01f) {
                    p.position.y = 0.01f;
                    p.velocity.y *= -0.35f;
                    p.velocity.x *= 0.7f;
                    p.velocity.z *= 0.7f;
                }
                break;
            case PARTICLE_SMOKE:
            case PARTICLE_STEAM:
                p.velocity.x *= (1.0f - 0.6f * dt);
                p.velocity.z *= (1.0f - 0.6f * dt);
                p.position += p.velocity * dt;
                p.position.x += std::sin(p.age * 3.0f + p.spiralAngle) * 0.08f * dt;
                break;
            case PARTICLE_ARCANE_SPARK: {
                p.spiralAngle += dt * 3.8f;
                float r = 0.22f + 0.20f * (p.age / p.lifetime);
                p.position.y += p.velocity.y * dt;
                p.position.x = flaskPos.x + std::cos(p.spiralAngle) * r;
                p.position.z = flaskPos.z + std::sin(p.spiralAngle) * r;
                break;
            }
            case PARTICLE_FROST_MIST:
                // Heavy cold cryo vapor sinks gently toward the tabletop and swirls outward
                p.velocity.y -= 0.42f * dt;
                p.velocity.x *= (1.0f - 0.45f * dt);
                p.velocity.z *= (1.0f - 0.45f * dt);
                p.position += p.velocity * dt;
                if (p.position.y < 0.03f) {
                    p.position.y = 0.03f;
                    p.velocity.y = 0.0f;
                }
                break;
            case PARTICLE_BUBBLE:
                p.position += p.velocity * dt;
                p.position.x += std::sin(p.age * 10.0f + p.spiralAngle) * 0.04f * dt;
                break;
            case PARTICLE_AMBIENT_DUST:
                p.position += p.velocity * dt;
                p.position.x += std::sin(p.age * 0.9f + p.spiralAngle) * 0.02f * dt;
                break;
        }
        ++i;
    }
}

void ParticleSystem::drawBatch(const std::vector<ParticleInstanceData>& batch) {
    if (batch.empty()) return;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch.size() * sizeof(ParticleInstanceData), batch.data());
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (GLsizei)batch.size());
    glBindVertexArray(0);
}

void ParticleSystem::render(const Mat4& view, const Mat4& proj) {
    if (particles.empty()) return;

    alphaBatch.clear();
    additiveBatch.clear();

    for (const auto& p : particles) {
        float t = std::max(0.0f, std::min(1.0f, p.age / p.lifetime));
        float size = p.sizeStart + (p.sizeEnd - p.sizeStart) * t;
        // Fade in quickly on spawn for smooth visual entry
        float fadeIn = std::min(1.0f, p.age * 8.0f);
        ParticleInstanceData inst;
        inst.x = p.position.x;
        inst.y = p.position.y;
        inst.z = p.position.z;
        inst.size = size;
        inst.r = p.colorStart.x + (p.colorEnd.x - p.colorStart.x) * t;
        inst.g = p.colorStart.y + (p.colorEnd.y - p.colorStart.y) * t;
        inst.b = p.colorStart.z + (p.colorEnd.z - p.colorStart.z) * t;
        inst.a = (p.colorStart.w + (p.colorEnd.w - p.colorStart.w) * t) * fadeIn;

        if (p.additive) additiveBatch.push_back(inst);
        else            alphaBatch.push_back(inst);
    }

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    particleTex.bind(0);
    shader.setInt("uParticleTex", 0);

    // Pass 1: Translucent steam, smoke, cryo-mist
    if (!alphaBatch.empty()) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawBatch(alphaBatch);
    }

    // Pass 2: Glowing bubbles, fiery embers, arcane sparks, golden dust
    if (!additiveBatch.empty()) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawBatch(additiveBatch);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
