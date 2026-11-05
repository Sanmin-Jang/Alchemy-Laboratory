#include "Scene.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>

Scene::Scene()
    : flaskPos(0.0f, 0.0f, 0.0f),
      isDragging(false),
      timeElapsed(0.0f),
      potionName("Alchemy Elixir"),
      colorName("Emerald"),
      currentColorIndex(1),
      targetLiquidColor(0.12f, 0.95f, 0.35f, 0.90f) {}

Scene::~Scene() {}

void Scene::init() {
    // 1. Compile and link modern GLSL shaders
    glassShader.load("shaders/flask_glass.vert", "shaders/flask_glass.frag");
    liquidShader.load("shaders/flask_liquid.vert", "shaders/flask_liquid.frag");
    corkShader.load("shaders/cork.vert", "shaders/cork.frag");

    // 2. Generate 3D geometry meshes (Erlenmeyer flask, liquid volume, top meniscus cap, cork stopper)
    flaskMesh = Mesh::createErlenmeyerFlask(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    liquidMesh = Mesh::createLiquidVolume(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    diskMesh = Mesh::createDisk(0.25f, 16, 48);
    corkMesh = Mesh::createCylinder(0.046f, 0.052f, 0.06f, 32, true, true);

    // 3. Initialize sloshing physics
    slosh.currentPos = flaskPos;
    slosh.prevPos = flaskPos;
    slosh.fillLevel = 0.60f;
    slosh.liquidColor = targetLiquidColor;
}

void Scene::update(float dt) {
    timeElapsed += dt;

    // Smooth color transition
    slosh.liquidColor = Vec4(
        Vec3::lerp(slosh.liquidColor.xyz(), targetLiquidColor.xyz(), dt * 4.5f),
        targetLiquidColor.w
    );

    // Update dynamic liquid slosh oscillator (acceleration -> inertia -> damped harmonic motion)
    slosh.update(flaskPos, dt);
}

void Scene::onFlaskDragStart() {
    isDragging = true;
    slosh.addImpulse(0.4f, 0.0f);
}

void Scene::onFlaskDrag(const Vec3& newPos) {
    flaskPos = newPos;
}

void Scene::onFlaskDragEnd() {
    isDragging = false;
    slosh.addImpulse(0.0f, 0.35f);
}

void Scene::adjustLiquidLevel(float delta) {
    slosh.fillLevel = std::max(0.10f, std::min(0.92f, slosh.fillLevel + delta));
    slosh.stir(0.4f);
}

void Scene::stirPotion() {
    slosh.stir(1.2f);
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
    slosh.fillLevel = 0.60f;
    setColorPreset(1);
}

void Scene::setColorPreset(int preset) {
    currentColorIndex = preset;
    switch (preset) {
        case 1: // Emerald Green
            targetLiquidColor = Vec4(0.12f, 0.95f, 0.35f, 0.90f);
            colorName = "Emerald";
            break;
        case 2: // Ruby Crimson
            targetLiquidColor = Vec4(0.98f, 0.12f, 0.25f, 0.90f);
            colorName = "Ruby";
            break;
        case 3: // Sapphire Blue
            targetLiquidColor = Vec4(0.12f, 0.65f, 0.98f, 0.90f);
            colorName = "Sapphire";
            break;
        case 4: // Amethyst Purple
            targetLiquidColor = Vec4(0.72f, 0.15f, 0.98f, 0.90f);
            colorName = "Amethyst";
            break;
        case 5: // Auric Gold
            targetLiquidColor = Vec4(0.98f, 0.82f, 0.12f, 0.90f);
            colorName = "Gold";
            break;
        case 6: // Frost Cyan
            targetLiquidColor = Vec4(0.12f, 0.96f, 0.92f, 0.90f);
            colorName = "Cyan";
            break;
        default:
            break;
    }
    slosh.stir(0.45f); // subtle ripple on chemical reaction
}

void Scene::cycleColor() {
    int nextPreset = (currentColorIndex % 6) + 1;
    setColorPreset(nextPreset);
}

float Scene::getFlaskRadiusAtHeight(float h) {
    if (h <= 0.02f) {
        return 0.22f - (0.22f - 0.209f) * (h / 0.02f);
    } else if (h <= 0.31f) {
        float t = (h - 0.02f) / (0.31f - 0.02f);
        return 0.209f + (0.085f - 0.209f) * t;
    } else if (h <= 0.34f) {
        float t = (h - 0.31f) / 0.03f;
        return 0.085f + (0.05f - 0.085f) * t;
    }
    return 0.05f;
}

bool Scene::hitTestFlask(const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) const {
    Vec3 flaskCenter = flaskPos + Vec3(0.0f, 0.23f, 0.0f);
    Vec3 oc = rayOrigin - flaskCenter;
    float b = Vec3::dot(oc, rayDir);
    float c = Vec3::dot(oc, oc) - 0.32f * 0.32f;
    float discriminant = b * b - c;
    if (discriminant > 0.0f) {
        outDist = -b - std::sqrt(discriminant);
        return outDist > 0.0f;
    }
    return false;
}

void Scene::render(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 1. Render cork stopper
    renderCork(view, proj, cameraPos);

    // 2. Render dynamic sloshing green liquid
    renderLiquid(view, proj, cameraPos);

    // 3. Render transparent glassware
    renderGlass(view, proj, cameraPos);
}

void Scene::renderLiquid(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    liquidShader.use();
    liquidShader.setMat4("uView", view);
    liquidShader.setMat4("uProjection", proj);
    liquidShader.setVec3("uCameraPos", cameraPos);

    liquidShader.setVec4("uLiquidColor", slosh.liquidColor);
    liquidShader.setFloat("uFillLevel", slosh.fillLevel);
    liquidShader.setVec3("uSloshNormal", slosh.getSurfaceNormal());
    liquidShader.setFloat("uFlaskMaxHeight", 0.46f);
    liquidShader.setFloat("uTime", timeElapsed);
    liquidShader.setFloat("uWaveAmp", slosh.waveAmplitude);
    liquidShader.setFloat("uWavePhase", slosh.wavePhase);

    // Both liquid volume body and top surface disk share the exact same model matrix
    Mat4 mLiquidModel = Mat4::translate(flaskPos);
    liquidShader.setMat4("uModel", mLiquidModel);
    float n3x3[9];
    mLiquidModel.getNormalMatrix(n3x3);
    liquidShader.setMat3("uNormalMatrix", n3x3);

    // Pass 1: Render main conical liquid body
    liquidShader.setBool("uIsTopCap", false);
    liquidMesh.draw();

    // Pass 2: Render top sloshing surface cap disk
    liquidShader.setBool("uIsTopCap", true);
    diskMesh.draw();

    glDisable(GL_BLEND);
}

void Scene::renderGlass(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    glassShader.use();
    glassShader.setMat4("uView", view);
    glassShader.setMat4("uProjection", proj);
    glassShader.setVec3("uCameraPos", cameraPos);

    Mat4 mFlask = Mat4::translate(flaskPos);
    glassShader.setMat4("uModel", mFlask);
    float n3x3[9];
    mFlask.getNormalMatrix(n3x3);
    glassShader.setMat3("uNormalMatrix", n3x3);

    // Pass 1: Render back-faces (inner glass reflections)
    glCullFace(GL_FRONT);
    glassShader.setBool("uIsBackPass", true);
    flaskMesh.draw();

    // Pass 2: Render front-faces (Fresnel highlights and outer reflections)
    glCullFace(GL_BACK);
    glassShader.setBool("uIsBackPass", false);
    flaskMesh.draw();

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Scene::renderCork(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    corkShader.use();
    corkShader.setMat4("uView", view);
    corkShader.setMat4("uProjection", proj);
    corkShader.setVec3("uCameraPos", cameraPos);

    // Cork sits in the neck mouth (hierarchical child of flaskPos)
    Mat4 mCork = Mat4::translate(flaskPos) * Mat4::translate(0.0f, 0.44f, 0.0f);
    corkShader.setMat4("uModel", mCork);
    float n3x3[9];
    mCork.getNormalMatrix(n3x3);
    corkShader.setMat3("uNormalMatrix", n3x3);

    corkMesh.draw();
}
