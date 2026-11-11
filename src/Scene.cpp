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
      targetLiquidColor(0.12f, 0.95f, 0.35f, 0.90f),
      candlesLit(true),
      candleFlicker0(1.0f),
      candleFlicker1(1.0f) {}

Scene::~Scene() {}

void Scene::init() {
    // 1. Compile and link modern GLSL shaders
    glassShader.load("shaders/flask_glass.vert", "shaders/flask_glass.frag");
    liquidShader.load("shaders/flask_liquid.vert", "shaders/flask_liquid.frag");
    corkShader.load("shaders/cork.vert", "shaders/cork.frag");
    standardShader.load("shaders/standard.vert", "shaders/standard.frag");
    flameShader.load("shaders/flame.vert", "shaders/flame.frag");
    runeShader.load("shaders/rune.vert", "shaders/rune.frag");
    glowShader.load("shaders/glow.vert", "shaders/glow.frag");

    // 2. Generate procedural textures
    woodTex = Texture::createWoodTexture(512);
    stoneTex = Texture::createStoneWallTexture(512);
    parchmentTex = Texture::createParchmentTexture(512);
    runeTex = Texture::createRuneCircleTexture(512);

    // 3. Generate glassware & liquid geometry meshes
    flaskMesh = Mesh::createErlenmeyerFlask(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    liquidMesh = Mesh::createLiquidVolume(0.22f, 0.085f, 0.05f, 0.46f, 0.15f, 48);
    diskMesh = Mesh::createDisk(0.25f, 16, 48);
    corkMesh = Mesh::createCylinder(0.046f, 0.052f, 0.06f, 32, true, true);

    // 4. Generate table and environment meshes
    tableTopMesh = Mesh::createBox(2.8f, 0.07f, 1.6f, 2.2f, 1.6f);
    tableLegMesh = Mesh::createBox(0.10f, 1.0f, 0.10f, 0.3f, 2.0f);
    tableApronLongMesh = Mesh::createBox(2.36f, 0.08f, 0.05f, 2.0f, 0.2f);
    tableApronShortMesh = Mesh::createBox(0.05f, 0.08f, 1.21f, 0.2f, 1.2f);
    floorMesh = Mesh::createPlane(14.0f, 14.0f, 7.0f, 7.0f);
    backWallMesh = Mesh::createPlane(14.0f, 5.0f, 7.0f, 3.0f);
    runePlaneMesh = Mesh::createPlane(0.82f, 0.82f, 1.0f, 1.0f);
    parchmentPlaneMesh = Mesh::createPlane(0.42f, 0.32f, 1.0f, 1.0f);

    // 5. Generate candle components
    candleWaxUnitMesh = Mesh::createCylinder(1.0f, 0.94f, 1.0f, 24, true, true);
    candleSaucerLeftMesh = Mesh::createCylinder(0.19f, 0.21f, 0.018f, 32, true, true);
    candleSaucerRightMesh = Mesh::createCylinder(0.15f, 0.17f, 0.016f, 28, true, true);
    candleWickMesh = Mesh::createCylinder(0.003f, 0.002f, 0.022f, 8, true, true);
    flameMesh = Mesh::createTeardropFlame(0.025f, 0.18f, 16);
    billboardMesh = Mesh::createBillboard(1.0f);
    waxDropMesh = Mesh::createSphere(0.022f, 10, 12);

    // 6. Define candle placement on the laboratory table
    candles.clear();
    // Left candle cluster (Trio on antique brass saucer)
    candles.push_back({ Vec3(-0.72f, 0.018f, -0.15f), 0.28f, 0.032f, 0.0f, 0.90f }); // Tall main candle
    candles.push_back({ Vec3(-0.63f, 0.018f, -0.07f), 0.18f, 0.029f, 2.7f, 0.78f }); // Medium candle
    candles.push_back({ Vec3(-0.80f, 0.018f, -0.22f), 0.11f, 0.033f, 5.1f, 0.68f }); // Short melted candle

    // Right candle cluster (Pair on brass/iron saucer)
    candles.push_back({ Vec3(0.73f, 0.016f, -0.18f), 0.23f, 0.028f, 1.4f, 0.84f }); // Medium slender candle
    candles.push_back({ Vec3(0.82f, 0.016f, -0.27f), 0.13f, 0.031f, 3.9f, 0.70f }); // Little stub candle

    // 7. Initialize sloshing physics
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

    // Dynamic candle flame flickers with superimposed non-repeating harmonics
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
    slosh.stir(0.45f); // subtle ripple on chemical transmutation
}

void Scene::cycleColor() {
    int nextPreset = (currentColorIndex % 6) + 1;
    setColorPreset(nextPreset);
}

void Scene::toggleCandles() {
    candlesLit = !candlesLit;
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

void Scene::setupLights(const Shader& shader, const Vec3& cameraPos) const {
    shader.setVec3("uViewPos", cameraPos);

    // Light 0: Left candle cluster
    shader.setVec3("uLights[0].position", Vec3(-0.71f, 0.35f, -0.15f));
    shader.setVec3("uLights[0].color", Vec3(1.0f, 0.74f, 0.38f));
    shader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0) : 0.0f);

    // Light 1: Right candle cluster
    shader.setVec3("uLights[1].position", Vec3(0.75f, 0.28f, -0.22f));
    shader.setVec3("uLights[1].color", Vec3(1.0f, 0.70f, 0.34f));
    shader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1) : 0.0f);

    // Light 2: Glowing potion elixir
    shader.setVec3("uLights[2].position", flaskPos + Vec3(0.0f, 0.18f, 0.0f));
    shader.setVec3("uLights[2].color", slosh.liquidColor.xyz() * 1.25f);
    float potionPulse = 0.85f + 0.20f * std::sin(timeElapsed * 3.5f);
    shader.setFloat("uLights[2].intensity", potionPulse);
}

void Scene::render(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // 1. Opaque laboratory environment: wooden workbench, legs, stone floor, back wall
    renderRoomAndTable(view, proj, cameraPos);

    // 2. Opaque candle bodies, brass saucers, charred wicks, melted wax drips
    renderCandles(view, proj, cameraPos);

    // 3. Ancient alchemy parchment scroll on table
    renderParchment(view, proj, cameraPos);

    // 4. Cork stopper
    renderCork(view, proj, cameraPos);

    // 5. Mystical glowing runic circle on tabletop
    renderRuneCircle(view, proj, cameraPos);

    // 6. Dynamic sloshing liquid volume
    renderLiquid(view, proj, cameraPos);

    // 7. Transparent glassware with Fresnel and candle reflections
    renderGlass(view, proj, cameraPos);

    // 8. Atmospheric candle flames, radial light glows, and potion aura
    renderFlamesAndGlow(view, proj, cameraPos);
}

void Scene::renderRoomAndTable(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    standardShader.use();
    standardShader.setMat4("uView", view);
    standardShader.setMat4("uProjection", proj);
    setupLights(standardShader, cameraPos);
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // 1. Table Top Slab
    {
        Mat4 m = Mat4::translate(0.0f, -0.035f, -0.15f);
        standardShader.setMat4("uModel", m);
        float n3x3[9];
        m.getNormalMatrix(n3x3);
        standardShader.setMat3("uNormalMatrix", n3x3);

        standardShader.setBool("uUseTexture", true);
        woodTex.bind(0);
        standardShader.setInt("uTexture", 0);

        standardShader.setVec3("uDiffuseColor", Vec3(1.0f, 0.95f, 0.90f));
        standardShader.setVec3("uSpecularColor", Vec3(0.18f, 0.14f, 0.10f));
        standardShader.setFloat("uShininess", 24.0f);

        tableTopMesh.draw();
    }

    // 2. Table Aprons (Support beams under tabletop)
    {
        standardShader.setVec3("uDiffuseColor", Vec3(0.85f, 0.80f, 0.75f));
        standardShader.setVec3("uSpecularColor", Vec3(0.12f, 0.10f, 0.08f));
        standardShader.setFloat("uShininess", 16.0f);

        // Front apron
        Mat4 mf = Mat4::translate(0.0f, -0.105f, 0.48f);
        standardShader.setMat4("uModel", mf);
        float nf[9]; mf.getNormalMatrix(nf);
        standardShader.setMat3("uNormalMatrix", nf);
        tableApronLongMesh.draw();

        // Back apron
        Mat4 mb = Mat4::translate(0.0f, -0.105f, -0.78f);
        standardShader.setMat4("uModel", mb);
        float nb[9]; mb.getNormalMatrix(nb);
        standardShader.setMat3("uNormalMatrix", nb);
        tableApronLongMesh.draw();

        // Left apron
        Mat4 ml = Mat4::translate(-1.18f, -0.105f, -0.15f);
        standardShader.setMat4("uModel", ml);
        float nl[9]; ml.getNormalMatrix(nl);
        standardShader.setMat3("uNormalMatrix", nl);
        tableApronShortMesh.draw();

        // Right apron
        Mat4 mr = Mat4::translate(1.18f, -0.105f, -0.15f);
        standardShader.setMat4("uModel", mr);
        float nr[9]; mr.getNormalMatrix(nr);
        standardShader.setMat3("uNormalMatrix", nr);
        tableApronShortMesh.draw();
    }

    // 3. Table Legs
    {
        standardShader.setVec3("uDiffuseColor", Vec3(0.90f, 0.85f, 0.80f));
        standardShader.setVec3("uSpecularColor", Vec3(0.14f, 0.12f, 0.10f));
        standardShader.setFloat("uShininess", 18.0f);

        const float legX[4] = { -1.18f,  1.18f, -1.18f,  1.18f };
        const float legZ[4] = {  0.48f,  0.48f, -0.78f, -0.78f };

        for (int i = 0; i < 4; ++i) {
            Mat4 m = Mat4::translate(legX[i], -0.57f, legZ[i]);
            standardShader.setMat4("uModel", m);
            float n3x3[9];
            m.getNormalMatrix(n3x3);
            standardShader.setMat3("uNormalMatrix", n3x3);
            tableLegMesh.draw();
        }
    }

    // 4. Laboratory Stone Floor
    {
        Mat4 m = Mat4::translate(0.0f, -1.07f, 0.0f);
        standardShader.setMat4("uModel", m);
        float n3x3[9];
        m.getNormalMatrix(n3x3);
        standardShader.setMat3("uNormalMatrix", n3x3);

        standardShader.setBool("uUseTexture", true);
        stoneTex.bind(0);
        standardShader.setInt("uTexture", 0);

        standardShader.setVec3("uDiffuseColor", Vec3(0.65f, 0.62f, 0.60f));
        standardShader.setVec3("uSpecularColor", Vec3(0.08f, 0.08f, 0.08f));
        standardShader.setFloat("uShininess", 8.0f);

        floorMesh.draw();
    }

    // 5. Laboratory Back Stone Wall
    {
        Mat4 m = Mat4::translate(0.0f, 1.20f, -2.20f) * Mat4::rotateX((float)M_PI * 0.5f);
        standardShader.setMat4("uModel", m);
        float n3x3[9];
        m.getNormalMatrix(n3x3);
        standardShader.setMat3("uNormalMatrix", n3x3);

        standardShader.setBool("uUseTexture", true);
        stoneTex.bind(0);
        standardShader.setInt("uTexture", 0);

        standardShader.setVec3("uDiffuseColor", Vec3(0.55f, 0.52f, 0.52f));
        standardShader.setVec3("uSpecularColor", Vec3(0.06f, 0.06f, 0.06f));
        standardShader.setFloat("uShininess", 6.0f);

        backWallMesh.draw();
    }
}

void Scene::renderCandles(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    standardShader.use();
    standardShader.setMat4("uView", view);
    standardShader.setMat4("uProjection", proj);
    setupLights(standardShader, cameraPos);

    standardShader.setBool("uUseTexture", false);
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // 1. Antique Brass Candle Saucers
    // Left brass saucer
    {
        Mat4 m = Mat4::translate(-0.72f, 0.0f, -0.15f);
        standardShader.setMat4("uModel", m);
        float n3x3[9]; m.getNormalMatrix(n3x3);
        standardShader.setMat3("uNormalMatrix", n3x3);

        standardShader.setVec3("uDiffuseColor", Vec3(0.72f, 0.58f, 0.26f));
        standardShader.setVec3("uSpecularColor", Vec3(0.95f, 0.85f, 0.55f));
        standardShader.setFloat("uShininess", 48.0f);

        candleSaucerLeftMesh.draw();
    }
    // Right brass saucer
    {
        Mat4 m = Mat4::translate(0.76f, 0.0f, -0.22f);
        standardShader.setMat4("uModel", m);
        float n3x3[9]; m.getNormalMatrix(n3x3);
        standardShader.setMat3("uNormalMatrix", n3x3);

        standardShader.setVec3("uDiffuseColor", Vec3(0.65f, 0.54f, 0.28f));
        standardShader.setVec3("uSpecularColor", Vec3(0.90f, 0.80f, 0.50f));
        standardShader.setFloat("uShininess", 40.0f);

        candleSaucerRightMesh.draw();
    }

    // 2. Candle Wax Bodies & Charred Wicks
    for (const auto& c : candles) {
        // Wax cylinder
        Mat4 mWax = Mat4::translate(c.position) * Mat4::scale(c.radius, c.height, c.radius);
        standardShader.setMat4("uModel", mWax);
        float nWax[9]; mWax.getNormalMatrix(nWax);
        standardShader.setMat3("uNormalMatrix", nWax);

        standardShader.setVec3("uDiffuseColor", Vec3(0.92f, 0.88f, 0.77f)); // creamy beeswax ivory
        standardShader.setVec3("uSpecularColor", Vec3(0.20f, 0.18f, 0.15f));
        standardShader.setFloat("uShininess", 12.0f);

        candleWaxUnitMesh.draw();

        // Charred dark candle wick
        Mat4 mWick = Mat4::translate(c.position + Vec3(0.0f, c.height, 0.0f));
        standardShader.setMat4("uModel", mWick);
        float nWick[9]; mWick.getNormalMatrix(nWick);
        standardShader.setMat3("uNormalMatrix", nWick);

        standardShader.setVec3("uDiffuseColor", Vec3(0.10f, 0.08f, 0.06f));
        standardShader.setVec3("uSpecularColor", Vec3(0.0f, 0.0f, 0.0f));
        standardShader.setFloat("uShininess", 1.0f);

        candleWickMesh.draw();
    }

    // 3. Melted wax beads around candle bases
    standardShader.setVec3("uDiffuseColor", Vec3(0.90f, 0.86f, 0.75f));
    standardShader.setVec3("uSpecularColor", Vec3(0.18f, 0.16f, 0.14f));
    standardShader.setFloat("uShininess", 10.0f);

    const Vec3 dripPositions[5] = {
        Vec3(-0.68f, 0.019f, -0.11f),
        Vec3(-0.76f, 0.019f, -0.19f),
        Vec3(-0.58f, 0.019f, -0.05f),
        Vec3( 0.70f, 0.017f, -0.15f),
        Vec3( 0.78f, 0.017f, -0.24f)
    };
    for (int i = 0; i < 5; ++i) {
        Mat4 mDrop = Mat4::translate(dripPositions[i]) * Mat4::scale(1.2f, 0.35f, 1.2f);
        standardShader.setMat4("uModel", mDrop);
        float nDrop[9]; mDrop.getNormalMatrix(nDrop);
        standardShader.setMat3("uNormalMatrix", nDrop);
        waxDropMesh.draw();
    }
}

void Scene::renderParchment(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    standardShader.use();
    standardShader.setMat4("uView", view);
    standardShader.setMat4("uProjection", proj);
    setupLights(standardShader, cameraPos);

    standardShader.setBool("uUseTexture", true);
    parchmentTex.bind(0);
    standardShader.setInt("uTexture", 0);

    Mat4 m = Mat4::translate(0.54f, 0.0015f, 0.20f) * Mat4::rotateY(-0.28f);
    standardShader.setMat4("uModel", m);
    float n3x3[9];
    m.getNormalMatrix(n3x3);
    standardShader.setMat3("uNormalMatrix", n3x3);

    standardShader.setVec3("uDiffuseColor", Vec3(1.0f, 0.98f, 0.92f));
    standardShader.setVec3("uSpecularColor", Vec3(0.04f, 0.04f, 0.04f));
    standardShader.setFloat("uShininess", 4.0f);
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    parchmentPlaneMesh.draw();
}

void Scene::renderRuneCircle(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for luminous arcane glow

    runeShader.use();
    runeShader.setMat4("uView", view);
    runeShader.setMat4("uProjection", proj);
    runeShader.setFloat("uTime", timeElapsed);

    float runeActivity = std::min(1.0f, slosh.waveAmplitude * 8.0f + (isDragging ? 0.35f : 0.0f));
    runeShader.setFloat("uActivity", runeActivity);
    runeShader.setVec3("uRuneColor", slosh.liquidColor.xyz());

    runeTex.bind(0);
    runeShader.setInt("uRuneTexture", 0);

    // Situated right beneath the flask's resting position on the table
    Mat4 m = Mat4::translate(0.0f, 0.001f, 0.0f);
    runeShader.setMat4("uModel", m);

    runePlaneMesh.draw();

    glDisable(GL_BLEND);
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

    // Candle lighting illumination on liquid surface
    liquidShader.setVec3("uLights[0].position", Vec3(-0.71f, 0.35f, -0.15f));
    liquidShader.setVec3("uLights[0].color", Vec3(1.0f, 0.74f, 0.38f));
    liquidShader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0) : 0.0f);

    liquidShader.setVec3("uLights[1].position", Vec3(0.75f, 0.28f, -0.22f));
    liquidShader.setVec3("uLights[1].color", Vec3(1.0f, 0.70f, 0.34f));
    liquidShader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1) : 0.0f);

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

    // Candle reflections glinting on glass
    glassShader.setVec3("uLights[0].position", Vec3(-0.71f, 0.35f, -0.15f));
    glassShader.setVec3("uLights[0].color", Vec3(1.0f, 0.74f, 0.38f));
    glassShader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0) : 0.0f);

    glassShader.setVec3("uLights[1].position", Vec3(0.75f, 0.28f, -0.22f));
    glassShader.setVec3("uLights[1].color", Vec3(1.0f, 0.70f, 0.34f));
    glassShader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1) : 0.0f);

    Mat4 mFlask = Mat4::translate(flaskPos);
    glassShader.setMat4("uModel", mFlask);
    float n3x3[9];
    mFlask.getNormalMatrix(n3x3);
    glassShader.setMat3("uNormalMatrix", n3x3);

    // Pass 1: Render back-faces (inner glass reflections)
    glCullFace(GL_FRONT);
    glassShader.setBool("uIsBackPass", true);
    flaskMesh.draw();

    // Pass 2: Render front-faces (Fresnel highlights, candle reflections, and laboratory specularity)
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

    // Candle lighting for cork stopper
    corkShader.setVec3("uLights[0].position", Vec3(-0.71f, 0.35f, -0.15f));
    corkShader.setVec3("uLights[0].color", Vec3(1.0f, 0.74f, 0.38f));
    corkShader.setFloat("uLights[0].intensity", candlesLit ? (1.35f * candleFlicker0) : 0.0f);

    corkShader.setVec3("uLights[1].position", Vec3(0.75f, 0.28f, -0.22f));
    corkShader.setVec3("uLights[1].color", Vec3(1.0f, 0.70f, 0.34f));
    corkShader.setFloat("uLights[1].intensity", candlesLit ? (1.15f * candleFlicker1) : 0.0f);

    // Cork sits in the neck mouth (hierarchical child of flaskPos)
    Mat4 mCork = Mat4::translate(flaskPos) * Mat4::translate(0.0f, 0.44f, 0.0f);
    corkShader.setMat4("uModel", mCork);
    float n3x3[9];
    mCork.getNormalMatrix(n3x3);
    corkShader.setMat3("uNormalMatrix", n3x3);

    corkMesh.draw();
}

void Scene::renderFlamesAndGlow(const Mat4& view, const Mat4& proj, const Vec3& cameraPos) {
    if (!candlesLit) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for radiant fire
    glDepthMask(GL_FALSE);             // Avoid clipping transparent fire geometry

    // 1. Render animated teardrop candle flames
    flameShader.use();
    flameShader.setMat4("uView", view);
    flameShader.setMat4("uProjection", proj);
    flameShader.setFloat("uTime", timeElapsed);

    for (const auto& c : candles) {
        Vec3 flamePos = c.position + Vec3(0.0f, c.height + 0.008f, 0.0f);
        Mat4 mFlame = Mat4::translate(flamePos) * Mat4::scale(c.flameScale);
        flameShader.setMat4("uModel", mFlame);
        flameShader.setFloat("uSeed", c.seed);
        flameMesh.draw();
    }

    // 2. Camera-facing billboard rotation matrix
    Mat4 billboardRot = Mat4::identity();
    billboardRot.at(0, 0) = view.at(0, 0); billboardRot.at(1, 0) = view.at(0, 1); billboardRot.at(2, 0) = view.at(0, 2);
    billboardRot.at(0, 1) = view.at(1, 0); billboardRot.at(1, 1) = view.at(1, 1); billboardRot.at(2, 1) = view.at(1, 2);
    billboardRot.at(0, 2) = view.at(2, 0); billboardRot.at(1, 2) = view.at(2, 1); billboardRot.at(2, 2) = view.at(2, 2);

    // 3. Render soft golden light glow halos around candle flames
    glowShader.use();
    glowShader.setMat4("uView", view);
    glowShader.setMat4("uProjection", proj);
    glowShader.setVec3("uGlowColor", Vec3(1.0f, 0.68f, 0.22f));

    for (size_t i = 0; i < candles.size(); ++i) {
        const auto& c = candles[i];
        float flicker = (i < 3) ? candleFlicker0 : candleFlicker1;
        Vec3 haloPos = c.position + Vec3(0.0f, c.height + 0.055f, 0.0f);
        float haloSize = 0.28f * c.flameScale;

        Mat4 mHalo = Mat4::translate(haloPos) * billboardRot * Mat4::scale(haloSize);
        glowShader.setMat4("uModel", mHalo);
        glowShader.setFloat("uIntensity", 0.70f * flicker);
        billboardMesh.draw();
    }

    // 4. Render inner ethereal aura inside the flask matching current potion color
    Vec3 potionHaloPos = flaskPos + Vec3(0.0f, 0.16f, 0.0f);
    float potionHaloSize = 0.44f;
    Mat4 mPotionHalo = Mat4::translate(potionHaloPos) * billboardRot * Mat4::scale(potionHaloSize);
    glowShader.setMat4("uModel", mPotionHalo);
    glowShader.setVec3("uGlowColor", slosh.liquidColor.xyz());
    float potionPulse = 0.50f + 0.18f * std::sin(timeElapsed * 3.5f);
    glowShader.setFloat("uIntensity", potionPulse);
    billboardMesh.draw();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
