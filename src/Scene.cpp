#include "Scene.h"
#include <algorithm>
#include <cmath>

Scene::Scene()
    : flaskPos(0.0f, 0.0f, 0.0f),
      isDragging(false),
      timeElapsed(0.0f),
      runeRotation(0.0f),
      targetLiquidColor(0.22f, 0.65f, 0.72f, 0.88f),
      candlesLit(true),
      candleFlicker0(1.0f),
      candleFlicker1(1.0f),
      bubbleTimer(0.0f),
      steamTimer(0.0f),
      effectParticleTimer(0.0f) {}

Scene::~Scene() {}

void Scene::init() {
    // 1. Load GLSL 330 core shaders
    glassShader.load("shaders/flask_glass.vert", "shaders/flask_glass.frag");
    liquidShader.load("shaders/flask_liquid.vert", "shaders/flask_liquid.frag");
    corkShader.load("shaders/cork.vert", "shaders/cork.frag");
    standardShader.load("shaders/standard.vert", "shaders/standard.frag");
    flameShader.load("shaders/flame.vert", "shaders/flame.frag");
    runeShader.load("shaders/rune.vert", "shaders/rune.frag");
    glowShader.load("shaders/glow.vert", "shaders/glow.frag");

    // 2. Initialize subsystems
    env.init();
    potions.init();
    particles.init();
    runeTex = Texture::createRuneCircleTexture(512);

    // 3. Build main Erlenmeyer flask & ingredient bottle geometry
    flaskMesh     = Mesh::createErlenmeyerFlask(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    liquidMesh    = Mesh::createLiquidVolume(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    diskMesh      = Mesh::createDisk(0.25f, 16, 48);
    corkMesh      = Mesh::createCylinder(0.046f, 0.052f, 0.06f, 32, true, true);
    runePlaneMesh = Mesh::createPlane(0.92f, 0.92f, 1.0f, 1.0f);
    shockwaveMesh = Mesh::createTorus(1.0f, 0.06f, 40, 12);

    // Smaller reagent vials for the 3 base ingredients
    smallBottleGlassMesh  = Mesh::createErlenmeyerFlask(0.095f, 0.045f, 0.026f, 0.25f, 0.08f, 28);
    smallBottleLiquidMesh = Mesh::createCylinder(0.086f, 0.052f, 0.145f, 24, true, true);
    smallBottleCorkMesh   = Mesh::createCylinder(0.024f, 0.028f, 0.038f, 16, true, true);

    // 4. Initial liquid state (calm pale azure water waiting for ingredients)
    slosh.currentPos = flaskPos;
    slosh.prevPos = flaskPos;
    slosh.fillLevel = 0.25f;
    slosh.liquidColor = targetLiquidColor;
}

float Scene::getMagicResonance() const {
    if (potions.activeEffect == EFFECT_ARCANE_NOVA) {
        return std::min(1.0f, 0.45f + potions.effectTimer / std::max(0.1f, potions.effectDuration));
    }
    if (potions.activeEffect == EFFECT_EXPLOSION || potions.activeEffect == EFFECT_GLACIAL_MIST) {
        return std::min(1.0f, 0.25f + (potions.effectTimer / std::max(0.1f, potions.effectDuration)) * 0.75f);
    }
    return potions.canBrew() ? 0.35f : 0.05f;
}

Vec3 Scene::getBackgroundFogColor() const {
    Vec3 base(0.035f, 0.040f, 0.068f);
    float t = (potions.effectDuration > 0.01f) ? (potions.effectTimer / potions.effectDuration) : 0.0f;
    if (potions.activeEffect == EFFECT_EXPLOSION) {
        return Vec3::lerp(base, Vec3(0.22f, 0.07f, 0.02f), std::min(1.0f, t * 1.4f));
    }
    if (potions.activeEffect == EFFECT_ARCANE_NOVA) {
        return Vec3::lerp(base, Vec3(0.14f, 0.06f, 0.22f), std::min(1.0f, t * 1.1f));
    }
    if (potions.activeEffect == EFFECT_GLACIAL_MIST) {
        return Vec3::lerp(base, Vec3(0.04f, 0.14f, 0.22f), std::min(1.0f, t * 1.2f));
    }
    return base;
}

Vec3 Scene::getCameraShakeOffset() const {
    if (potions.shakeTimer <= 0.001f) return Vec3(0.0f, 0.0f, 0.0f);
    float amp = potions.shakeMagnitude * (potions.shakeTimer / 0.70f);
    return Vec3(
        std::sin(timeElapsed * 58.0f) * amp,
        std::cos(timeElapsed * 71.0f) * amp * 0.7f,
        std::sin(timeElapsed * 47.0f) * amp * 0.5f
    );
}

void Scene::update(float dt) {
    timeElapsed += dt;

    // Smooth liquid color transmutation
    slosh.liquidColor = Vec4(
        Vec3::lerp(slosh.liquidColor.xyz(), targetLiquidColor.xyz(), std::min(1.0f, dt * 4.5f)),
        targetLiquidColor.w
    );

    // Non-repeating multi-harmonic candle flickers
    candleFlicker0 = 1.0f
        + 0.14f * std::sin(timeElapsed * 11.3f)
        + 0.09f * std::sin(timeElapsed * 23.7f)
        + 0.06f * std::cos(timeElapsed * 41.1f)
        + 0.04f * std::sin(timeElapsed * 7.1f);

    candleFlicker1 = 1.0f
        + 0.13f * std::sin(timeElapsed * 13.9f + 1.7f)
        + 0.08f * std::sin(timeElapsed * 27.1f + 0.8f)
        + 0.06f * std::cos(timeElapsed * 37.3f + 2.3f)
        + 0.04f * std::sin(timeElapsed * 8.3f + 1.1f);

    // Update subsystems
    potions.update(dt);
    float resonance = getMagicResonance();
    env.update(dt, resonance);
    slosh.update(flaskPos, dt);
    particles.update(dt, flaskPos);

    // Rotate glowing magic circle faster during active recipe resonance
    runeRotation += dt * (0.28f + resonance * 1.85f);

    // Continuous bubbles & steam particles (Stretch Goal!)
    Vec3 mouthPos = flaskPos + Vec3(0.0f, 0.47f, 0.0f);
    float liquidTopY = 0.46f * slosh.fillLevel;

    bubbleTimer += dt;
    float bubbleInterval = (potions.activeEffect != EFFECT_NONE) ? 0.045f : 0.16f;
    if (bubbleTimer >= bubbleInterval && slosh.fillLevel > 0.18f) {
        bubbleTimer = 0.0f;
        particles.emitBubble(flaskPos, liquidTopY, slosh.liquidColor.xyz());
    }

    steamTimer += dt;
    float steamInterval = (potions.activeEffect != EFFECT_NONE || potions.canBrew()) ? 0.07f : 0.24f;
    if (steamTimer >= steamInterval && slosh.fillLevel > 0.30f) {
        steamTimer = 0.0f;
        particles.emitSteam(mouthPos, slosh.liquidColor.xyz(), 1);
    }

    // Continuous reaction particles for active brewed combinations
    if (potions.effectTimer > 0.0f) {
        effectParticleTimer += dt;
        if (potions.activeEffect == EFFECT_ARCANE_NOVA && effectParticleTimer >= 0.04f) {
            effectParticleTimer = 0.0f;
            particles.emitArcaneSpiral(flaskPos, 3);
        } else if (potions.activeEffect == EFFECT_GLACIAL_MIST && effectParticleTimer >= 0.045f) {
            effectParticleTimer = 0.0f;
            particles.emitFrostMist(flaskPos, 4);
        } else if (potions.activeEffect == EFFECT_EXPLOSION && potions.effectTimer > 2.0f && effectParticleTimer >= 0.08f) {
            effectParticleTimer = 0.0f;
            particles.emitSteam(mouthPos, Vec3(0.95f, 0.42f, 0.12f), 2);
        }
    }
}

bool Scene::addIngredient(IngredientType type) {
    Vec3 mixedColor;
    float newFill = slosh.fillLevel;
    if (!potions.addIngredient(type, mixedColor, newFill)) return false;

    targetLiquidColor = Vec4(mixedColor, 0.90f);
    slosh.fillLevel = newFill;
    slosh.stir(0.75f);
    slosh.boilIntensity = 0.45f;

    Vec3 mouthPos = flaskPos + Vec3(0.0f, 0.47f, 0.0f);
    particles.emitSplash(mouthPos, PotionSystem::getIngredientColor(type), 16);
    particles.emitSteam(mouthPos, mixedColor, 4);
    return true;
}

PotionEffect Scene::brewPotion() {
    Vec3 resultColor;
    PotionEffect eff = potions.brew(resultColor);
    if (eff == EFFECT_NONE) return EFFECT_NONE;

    targetLiquidColor = Vec4(resultColor, 0.92f);
    slosh.liquidColor = Vec4(Vec3::lerp(slosh.liquidColor.xyz(), resultColor, 0.78f), 0.92f);
    slosh.stir(1.5f);
    slosh.boilIntensity = 1.0f;

    Vec3 mouthPos = flaskPos + Vec3(0.0f, 0.47f, 0.0f);
    if (eff == EFFECT_EXPLOSION) {
        particles.triggerExplosionBurst(mouthPos);
    } else if (eff == EFFECT_ARCANE_NOVA) {
        particles.emitArcaneSpiral(flaskPos, 48);
        particles.emitSplash(mouthPos, Vec3(1.0f, 0.85f, 0.35f), 32);
    } else if (eff == EFFECT_GLACIAL_MIST) {
        particles.emitFrostMist(flaskPos, 60);
    } else {
        particles.emitSteam(mouthPos, resultColor, 14);
        particles.emitSplash(mouthPos, resultColor, 18);
    }
    return eff;
}

void Scene::onFlaskDragStart() { isDragging = true; slosh.addImpulse(0.4f, 0.0f); }
void Scene::onFlaskDrag(const Vec3& newPos) { flaskPos = newPos; }
void Scene::onFlaskDragEnd() { isDragging = false; slosh.addImpulse(0.0f, 0.35f); }
void Scene::adjustLiquidLevel(float delta) {
    slosh.fillLevel = std::max(0.12f, std::min(0.92f, slosh.fillLevel + delta));
    slosh.stir(0.4f);
}
void Scene::stirPotion() {
    slosh.stir(1.2f);
    slosh.boilIntensity = std::max(slosh.boilIntensity, 0.55f);
    particles.emitSteam(flaskPos + Vec3(0.0f, 0.47f, 0.0f), slosh.liquidColor.xyz(), 4);
}
void Scene::resetPotion() {
    flaskPos = Vec3(0.0f, 0.0f, 0.0f);
    slosh.currentPos = flaskPos;
    slosh.prevPos = flaskPos;
    slosh.velocity = Vec3(0.0f, 0.0f, 0.0f);
    slosh.acceleration = Vec3(0.0f, 0.0f, 0.0f);
    slosh.tilt = Vec2(0.0f, 0.0f);
    slosh.tiltVelocity = Vec2(0.0f, 0.0f);
    slosh.waveAmplitude = 0.0f;
    slosh.boilIntensity = 0.0f;
    slosh.fillLevel = 0.25f;
    targetLiquidColor = Vec4(0.22f, 0.65f, 0.72f, 0.88f);
    slosh.liquidColor = targetLiquidColor;
    particles.clearReactionParticles();
    potions.clearFlask();
}
void Scene::toggleCandles() { candlesLit = !candlesLit; }

bool Scene::hitTestFlask(const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) const {
    Vec3 flaskCenter = flaskPos + Vec3(0.0f, 0.23f, 0.0f);
    Vec3 oc = rayOrigin - flaskCenter;
    float b = Vec3::dot(oc, rayDir);
    float c = Vec3::dot(oc, oc) - 0.28f * 0.28f;
    float disc = b * b - c;
    if (disc > 0.0f) {
        outDist = -b - std::sqrt(disc);
        return outDist > 0.0f;
    }
    return false;
}

IngredientType Scene::hitTestIngredient(const Vec3& rayOrigin, const Vec3& rayDir) const {
    return potions.hitTestBottle(rayOrigin, rayDir);
}

void Scene::setupStandardLights(const Shader& shader, const Vec3& cameraPos) const {
    shader.setVec3("uViewPos", cameraPos);

    bool isFrost = (potions.activeEffect == EFFECT_GLACIAL_MIST && potions.effectTimer > 0.0f);
    float frostFactor = isFrost ? std::min(1.0f, potions.effectTimer / 3.0f) : 0.0f;
    float candleDim = 1.0f - 0.55f * frostFactor;
    Vec3 candleCol0 = Vec3::lerp(Vec3(1.0f, 0.74f, 0.38f), Vec3(0.45f, 0.85f, 1.0f), frostFactor * 0.65f);
    Vec3 candleCol1 = Vec3::lerp(Vec3(1.0f, 0.70f, 0.34f), Vec3(0.45f, 0.82f, 1.0f), frostFactor * 0.65f);

    // Light 0: Left flickering candle cluster
    shader.setVec3("uLights[0].position", Vec3(-0.87f, 0.36f, -0.18f));
    shader.setVec3("uLights[0].color", candleCol0);
    shader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0 * candleDim) : 0.0f);

    // Light 1: Right flickering candle cluster
    shader.setVec3("uLights[1].position", Vec3(0.90f, 0.29f, -0.22f));
    shader.setVec3("uLights[1].color", candleCol1);
    shader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1 * candleDim) : 0.0f);

    // Light 2: Dynamic Glowing Potion Light (surges powerfully on completed recipe = WOW MOMENT!)
    float potionIntensity = 0.85f + 0.18f * std::sin(timeElapsed * 3.5f);
    if (potions.activeEffect == EFFECT_ARCANE_NOVA) {
        float t = std::min(1.0f, potions.effectTimer / 2.5f);
        potionIntensity = 3.85f + 0.65f * std::sin(timeElapsed * 6.0f) * t;
    } else if (potions.activeEffect == EFFECT_EXPLOSION && potions.effectTimer > 3.5f) {
        float flash = (potions.effectTimer - 3.5f) / 2.0f;
        potionIntensity = 1.6f + 4.8f * flash * flash;
    } else if (potions.activeEffect == EFFECT_GLACIAL_MIST) {
        potionIntensity = 2.15f + 0.25f * std::sin(timeElapsed * 4.0f);
    }
    shader.setVec3("uLights[2].position", flaskPos + Vec3(0.0f, 0.24f, 0.0f));
    shader.setVec3("uLights[2].color", slosh.liquidColor.xyz() * 1.25f);
    shader.setFloat("uLights[2].intensity", potionIntensity);

    // Light 3: Atmospheric Gothic Window Moonlight (cool rim light from back wall)
    shader.setVec3("uLights[3].position", Vec3(0.65f, 1.45f, -1.90f));
    shader.setVec3("uLights[3].color", Vec3(0.45f, 0.62f, 1.0f));
    shader.setFloat("uLights[3].intensity", 0.95f);

    // Atmospheric dungeon fog & ambient
    shader.setVec3("uFogColor", getBackgroundFogColor());
    shader.setFloat("uFogDensity", isFrost ? 0.19f : 0.105f);
    Vec3 ambientBase(0.085f, 0.088f, 0.115f);
    if (potions.activeEffect == EFFECT_ARCANE_NOVA) ambientBase = Vec3(0.16f, 0.11f, 0.22f);
    else if (isFrost) ambientBase = Vec3(0.07f, 0.14f, 0.20f);
    shader.setVec3("uAmbientColor", ambientBase);
}

void Scene::setupCandleLights(const Shader& shader) const {
    shader.setVec3("uLights[0].position", Vec3(-0.87f, 0.36f, -0.18f));
    shader.setVec3("uLights[0].color", Vec3(1.0f, 0.74f, 0.38f));
    shader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0) : 0.0f);
    shader.setVec3("uLights[1].position", Vec3(0.90f, 0.29f, -0.22f));
    shader.setVec3("uLights[1].color", Vec3(1.0f, 0.70f, 0.34f));
    shader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1) : 0.0f);
}

void Scene::render(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // 1. Opaque Dungeon Room, Workbench, Books, Shelf, Astrolabe, Candles, Parchment
    standardShader.use();
    standardShader.setMat4("uView", view);
    standardShader.setMat4("uProjection", proj);
    setupStandardLights(standardShader, cameraPos);

    env.renderRoomAndTable(standardShader);
    env.renderLabProps(standardShader, slosh.liquidColor.xyz(), getMagicResonance());
    env.renderCandles(standardShader);
    env.renderParchment(standardShader);

    // 2. Ingredient Bottles (opaque liquid & cork + semi-transparent glass)
    renderIngredientBottles(view, proj, cameraPos);

    // 3. Main Flask Cork (pops & tumbles during Explosion)
    renderCork(view, proj, cameraPos);

    // 4. Glowing Rotating Magic Rune Circle on Tabletop
    renderRuneCircle(view, proj);

    // 5. Dynamic Sloshing Liquid Volume in Main Flask
    renderLiquid(view, proj, cameraPos);

    // 6. Semi-Transparent Fresnel Glass on Main Flask
    renderGlass(view, proj, cameraPos);

    // 7. Explosion Shockwave Ring + Candle Flames + Radial Glow Halos
    renderShockwave(view, proj);
    bool isFrost = (potions.activeEffect == EFFECT_GLACIAL_MIST && potions.effectTimer > 0.0f);
    env.renderFlamesAndGlow(flameShader, glowShader, view, proj, timeElapsed,
                            candlesLit, isFrost, candleFlicker0, candleFlicker1,
                            flaskPos, slosh.liquidColor.xyz(), getMagicResonance());

    // 8. GPU-Instanced 3D Particle System (Bubbles, Steam, Embers, Smoke, Arcane, Frost, Dust)
    particles.render(view, proj);
}

static void uploadModelAndNormal(const Shader& s, const Mat4& m) {
    s.setMat4("uModel", m);
    float n3[9]; m.getNormalMatrix(n3);
    s.setMat3("uNormalMatrix", n3);
}

void Scene::renderIngredientBottles(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    standardShader.use();
    standardShader.setMat4("uView", view);
    standardShader.setMat4("uProjection", proj);
    setupStandardLights(standardShader, cameraPos);
    standardShader.setBool("uUseTexture", false);

    for (size_t i = 0; i < potions.bottles.size(); ++i) {
        const auto& b = potions.bottles[i];
        Mat4 mParent = potions.getBottleModelMatrix(i, flaskPos);

        // Child 1: Glowing liquid volume
        uploadModelAndNormal(standardShader, mParent * Mat4::translate(0.0f, 0.008f, 0.0f));
        standardShader.setVec3("uDiffuseColor", b.color);
        standardShader.setVec3("uSpecularColor", Vec3(0.85f, 0.85f, 0.90f));
        standardShader.setFloat("uShininess", 48.0f);
        standardShader.setVec3("uEmissive", b.color * 0.42f);
        smallBottleLiquidMesh.draw();

        // Child 2: Cork stopper
        uploadModelAndNormal(standardShader, mParent * Mat4::translate(0.0f, 0.235f, 0.0f));
        standardShader.setVec3("uDiffuseColor", Vec3(0.65f, 0.46f, 0.28f));
        standardShader.setVec3("uSpecularColor", Vec3(0.06f, 0.05f, 0.04f));
        standardShader.setFloat("uShininess", 6.0f);
        standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));
        smallBottleCorkMesh.draw();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    glassShader.use();
    glassShader.setMat4("uView", view);
    glassShader.setMat4("uProjection", proj);
    glassShader.setVec3("uCameraPos", cameraPos);
    setupCandleLights(glassShader);

    for (size_t i = 0; i < potions.bottles.size(); ++i) {
        uploadModelAndNormal(glassShader, potions.getBottleModelMatrix(i, flaskPos));
        glCullFace(GL_FRONT); glassShader.setBool("uIsBackPass", true);  smallBottleGlassMesh.draw();
        glCullFace(GL_BACK);  glassShader.setBool("uIsBackPass", false); smallBottleGlassMesh.draw();
    }
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Scene::renderCork(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    corkShader.use();
    corkShader.setMat4("uView", view);
    corkShader.setMat4("uProjection", proj);
    corkShader.setVec3("uCameraPos", cameraPos);
    setupCandleLights(corkShader);

    Mat4 mCork = Mat4::translate(flaskPos) * Mat4::translate(0.0f, 0.44f, 0.0f)
               * Mat4::translate(potions.corkOffset)
               * Mat4::rotateX(potions.corkSpinAngle) * Mat4::rotateZ(potions.corkSpinAngle * 0.7f);
    uploadModelAndNormal(corkShader, mCork);
    corkMesh.draw();
}

void Scene::renderRuneCircle(const Mat4& view, const Mat4& proj) {
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); glDepthMask(GL_FALSE);
    runeShader.use();
    runeShader.setMat4("uView", view); runeShader.setMat4("uProjection", proj);
    runeShader.setFloat("uTime", timeElapsed); runeShader.setFloat("uRotation", runeRotation);
    float activity = std::min(1.5f, slosh.waveAmplitude * 7.0f + getMagicResonance() * 1.15f + (isDragging ? 0.3f : 0.0f));
    runeShader.setFloat("uActivity", activity);
    runeShader.setVec3("uRuneColor", slosh.liquidColor.xyz());
    runeTex.bind(0); runeShader.setInt("uRuneTexture", 0);
    runeShader.setMat4("uModel", Mat4::translate(0.0f, 0.002f, 0.0f));
    runePlaneMesh.draw();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);
}

void Scene::renderLiquid(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    liquidShader.use();
    liquidShader.setMat4("uView", view); liquidShader.setMat4("uProjection", proj);
    liquidShader.setVec3("uCameraPos", cameraPos);
    liquidShader.setVec4("uLiquidColor", slosh.liquidColor);
    liquidShader.setFloat("uFillLevel", slosh.fillLevel);
    liquidShader.setVec3("uSloshNormal", slosh.getSurfaceNormal());
    liquidShader.setFloat("uFlaskMaxHeight", 0.46f);
    liquidShader.setFloat("uTime", timeElapsed);
    liquidShader.setFloat("uWaveAmp", slosh.waveAmplitude);
    liquidShader.setFloat("uWavePhase", slosh.wavePhase);
    setupCandleLights(liquidShader);
    uploadModelAndNormal(liquidShader, Mat4::translate(flaskPos));
    liquidShader.setBool("uIsTopCap", false); liquidMesh.draw();
    liquidShader.setBool("uIsTopCap", true);  diskMesh.draw();
    glDisable(GL_BLEND);
}

void Scene::renderGlass(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glEnable(GL_CULL_FACE);
    glassShader.use();
    glassShader.setMat4("uView", view); glassShader.setMat4("uProjection", proj);
    glassShader.setVec3("uCameraPos", cameraPos);
    setupCandleLights(glassShader);
    uploadModelAndNormal(glassShader, Mat4::translate(flaskPos));
    glCullFace(GL_FRONT); glassShader.setBool("uIsBackPass", true);  flaskMesh.draw();
    glCullFace(GL_BACK);  glassShader.setBool("uIsBackPass", false); flaskMesh.draw();
    glDisable(GL_CULL_FACE); glDisable(GL_BLEND);
}

void Scene::renderShockwave(const Mat4& view, const Mat4& proj) {
    if (potions.shockwaveAlpha <= 0.01f) return;
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); glDepthMask(GL_FALSE);
    standardShader.use();
    standardShader.setMat4("uView", view); standardShader.setMat4("uProjection", proj);
    standardShader.setBool("uUseTexture", false);
    standardShader.setVec3("uDiffuseColor", Vec3(0.0f, 0.0f, 0.0f));
    standardShader.setVec3("uSpecularColor", Vec3(0.0f, 0.0f, 0.0f));
    standardShader.setFloat("uShininess", 1.0f);
    standardShader.setVec3("uEmissive", Vec3(1.0f, 0.45f, 0.08f) * (potions.shockwaveAlpha * 2.2f));
    uploadModelAndNormal(standardShader, Mat4::translate(flaskPos + Vec3(0.0f, 0.14f, 0.0f))
                                       * Mat4::scale(potions.shockwaveRadius, 0.35f, potions.shockwaveRadius));
    shockwaveMesh.draw();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);
}
