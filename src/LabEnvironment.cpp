#include "LabEnvironment.h"
#include <cmath>

LabEnvironment::LabEnvironment() : astrolabeAngle(0.0f) {}
LabEnvironment::~LabEnvironment() {}

void LabEnvironment::setModelAndNormal(const Shader& shader, const Mat4& model) const {
    shader.setMat4("uModel", model);
    float n3x3[9];
    model.getNormalMatrix(n3x3);
    shader.setMat3("uNormalMatrix", n3x3);
}

void LabEnvironment::init() {
    // 1. Procedural mipmapped textures
    woodTex        = Texture::createWoodTexture(512);
    stoneTex       = Texture::createStoneWallTexture(512);
    parchmentTex   = Texture::createParchmentTexture(512);
    mortarTex      = Texture::createMortarStoneTexture(256);
    bookLeatherTex = Texture::createBookLeatherTexture(256);
    windowTex      = Texture::createWindowTexture(256);

    // 2. Workbench & enclosed stone dungeon room
    tableTopMesh        = Mesh::createBox(2.85f, 0.075f, 1.65f, 2.4f, 1.6f);
    tableLegMesh        = Mesh::createBox(0.11f, 1.02f, 0.11f, 0.3f, 2.0f);
    tableApronLongMesh  = Mesh::createBox(2.40f, 0.085f, 0.055f, 2.0f, 0.2f);
    tableApronShortMesh = Mesh::createBox(0.055f, 0.085f, 1.24f, 0.2f, 1.2f);

    floorMesh    = Mesh::createPlane(8.6f, 7.2f, 6.0f, 5.2f);
    wallWideMesh = Mesh::createPlane(8.4f, 4.5f, 5.5f, 2.8f);
    wallSideMesh = Mesh::createPlane(6.8f, 4.5f, 4.6f, 2.8f);
    beamMesh     = Mesh::createBox(8.2f, 0.18f, 0.22f, 4.0f, 0.4f);
    pillarMesh   = Mesh::createBox(0.32f, 4.4f, 0.32f, 0.6f, 3.0f);
    windowMesh   = Mesh::createPlane(1.15f, 1.75f, 1.0f, 1.0f);
    parchmentPlaneMesh = Mesh::createPlane(0.46f, 0.34f, 1.0f, 1.0f);

    // 3. Spellbooks, shelf, mortar & pestle, apothecary jars
    bookCoverMesh    = Mesh::createBox(0.34f, 0.055f, 0.25f, 1.0f, 1.0f);
    bookPagesMesh    = Mesh::createBox(0.32f, 0.042f, 0.23f, 1.0f, 1.0f);
    shelfBoardMesh   = Mesh::createBox(1.55f, 0.05f, 0.28f, 1.5f, 0.4f);
    shelfBracketMesh = Mesh::createBox(0.05f, 0.22f, 0.24f, 0.2f, 0.5f);
    mortarBowlMesh   = Mesh::createCylinder(0.085f, 0.125f, 0.11f, 24, true, true);
    pestleMesh       = Mesh::createCylinder(0.018f, 0.028f, 0.19f, 14, true, true);
    jarMesh          = Mesh::createCylinder(0.055f, 0.048f, 0.16f, 18, true, true);

    // 4. Overhead Brass Alchemical Astrolabe (3-level parent-child hierarchy)
    astrolabeRodMesh   = Mesh::createCylinder(0.014f, 0.014f, 1.72f, 12, true, true);
    astrolabeOuterRing = Mesh::createTorus(0.32f, 0.015f, 36, 14);
    astrolabeInnerRing = Mesh::createTorus(0.21f, 0.012f, 32, 12);
    astrolabeCrystal   = Mesh::createSphere(0.058f, 12, 16);

    // 5. Candle clusters
    candleWaxUnitMesh     = Mesh::createCylinder(1.0f, 0.94f, 1.0f, 24, true, true);
    candleSaucerLeftMesh  = Mesh::createCylinder(0.19f, 0.21f, 0.018f, 32, true, true);
    candleSaucerRightMesh = Mesh::createCylinder(0.15f, 0.17f, 0.016f, 28, true, true);
    candleWickMesh        = Mesh::createCylinder(0.003f, 0.002f, 0.022f, 8, true, true);
    flameMesh             = Mesh::createTeardropFlame(0.025f, 0.18f, 16);
    billboardMesh         = Mesh::createBillboard(1.0f);
    waxDropMesh           = Mesh::createSphere(0.022f, 10, 12);

    candles.clear();
    // Left brass saucer trio
    candles.push_back({ Vec3(-0.88f, 0.018f, -0.18f), 0.29f, 0.032f, 0.0f, 0.92f });
    candles.push_back({ Vec3(-0.79f, 0.018f, -0.09f), 0.19f, 0.029f, 2.7f, 0.78f });
    candles.push_back({ Vec3(-0.96f, 0.018f, -0.25f), 0.12f, 0.033f, 5.1f, 0.68f });
    // Right brass saucer pair
    candles.push_back({ Vec3( 0.87f, 0.016f, -0.19f), 0.24f, 0.028f, 1.4f, 0.84f });
    candles.push_back({ Vec3( 0.96f, 0.016f, -0.28f), 0.14f, 0.031f, 3.9f, 0.70f });
}

void LabEnvironment::update(float dt, float magicResonance) {
    float speed = 0.55f + magicResonance * 2.4f;
    astrolabeAngle += dt * speed;
}

void LabEnvironment::renderRoomAndTable(const Shader& standardShader) const {
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // --- 1. Oak Workbench ---
    standardShader.setBool("uUseTexture", true);
    woodTex.bind(0);
    standardShader.setInt("uTexture", 0);

    // Tabletop slab
    setModelAndNormal(standardShader, Mat4::translate(0.0f, -0.0375f, -0.15f));
    standardShader.setVec3("uDiffuseColor", Vec3(1.0f, 0.95f, 0.88f));
    standardShader.setVec3("uSpecularColor", Vec3(0.20f, 0.15f, 0.10f));
    standardShader.setFloat("uShininess", 26.0f);
    tableTopMesh.draw();

    // Aprons
    standardShader.setVec3("uDiffuseColor", Vec3(0.84f, 0.78f, 0.72f));
    standardShader.setVec3("uSpecularColor", Vec3(0.12f, 0.10f, 0.08f));
    standardShader.setFloat("uShininess", 16.0f);
    setModelAndNormal(standardShader, Mat4::translate(0.0f, -0.11f,  0.50f)); tableApronLongMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate(0.0f, -0.11f, -0.80f)); tableApronLongMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate(-1.20f, -0.11f, -0.15f)); tableApronShortMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate( 1.20f, -0.11f, -0.15f)); tableApronShortMesh.draw();

    // 4 Table Legs
    const float legX[4] = { -1.20f,  1.20f, -1.20f,  1.20f };
    const float legZ[4] = {  0.50f,  0.50f, -0.80f, -0.80f };
    for (int i = 0; i < 4; ++i) {
        setModelAndNormal(standardShader, Mat4::translate(legX[i], -0.58f, legZ[i]));
        tableLegMesh.draw();
    }

    // Ceiling timber beams
    standardShader.setVec3("uDiffuseColor", Vec3(0.55f, 0.48f, 0.42f));
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 2.75f, -1.4f)); beamMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 2.75f,  0.2f)); beamMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 2.75f,  1.8f)); beamMesh.draw();

    // --- 2. Stone Dungeon Room (Floor, 4 Walls, Stone Pillars) ---
    stoneTex.bind(0);
    standardShader.setVec3("uDiffuseColor", Vec3(0.66f, 0.64f, 0.66f));
    standardShader.setVec3("uSpecularColor", Vec3(0.08f, 0.08f, 0.10f));
    standardShader.setFloat("uShininess", 8.0f);

    // Floor
    setModelAndNormal(standardShader, Mat4::translate(0.0f, -1.09f, 0.7f));
    floorMesh.draw();

    // Ceiling
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 2.85f, 0.7f) * Mat4::rotateX((float)M_PI));
    floorMesh.draw();

    // Back wall (-Z)
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 0.90f, -2.35f) * Mat4::rotateX((float)M_PI * 0.5f));
    wallWideMesh.draw();

    // Front wall (+Z behind player)
    setModelAndNormal(standardShader, Mat4::translate(0.0f, 0.90f, 3.85f) * Mat4::rotateX(-(float)M_PI * 0.5f));
    wallWideMesh.draw();

    // Left wall (-X): rotateY(+PI/2) * rotateX(+PI/2) faces +X with horizontal masonry & zero corner gaps
    setModelAndNormal(standardShader, Mat4::translate(-3.85f, 0.90f, 0.75f)
                                    * Mat4::rotateY((float)M_PI * 0.5f)
                                    * Mat4::rotateX((float)M_PI * 0.5f));
    wallSideMesh.draw();

    // Right wall (+X): rotateY(-PI/2) * rotateX(+PI/2) faces -X with horizontal masonry & zero corner gaps
    setModelAndNormal(standardShader, Mat4::translate(3.85f, 0.90f, 0.75f)
                                    * Mat4::rotateY(-(float)M_PI * 0.5f)
                                    * Mat4::rotateX((float)M_PI * 0.5f));
    wallSideMesh.draw();

    // Stone corner pillars framing all four corners + back archway
    const float pillarX[4] = { -3.72f, -1.55f, 1.55f, 3.72f };
    for (int i = 0; i < 4; ++i) {
        setModelAndNormal(standardShader, Mat4::translate(pillarX[i], 0.90f, -2.22f));
        pillarMesh.draw();
    }
    setModelAndNormal(standardShader, Mat4::translate(-3.72f, 0.90f, 3.72f)); pillarMesh.draw();
    setModelAndNormal(standardShader, Mat4::translate( 3.72f, 0.90f, 3.72f)); pillarMesh.draw();

    // --- 3. Gothic Moonlit Window on Back Wall ---
    windowTex.bind(0);
    setModelAndNormal(standardShader, Mat4::translate(0.65f, 1.25f, -2.31f) * Mat4::rotateX((float)M_PI * 0.5f));
    standardShader.setVec3("uDiffuseColor", Vec3(0.85f, 0.92f, 1.0f));
    standardShader.setVec3("uSpecularColor", Vec3(0.2f, 0.25f, 0.35f));
    standardShader.setFloat("uShininess", 24.0f);
    standardShader.setVec3("uEmissive", Vec3(0.18f, 0.26f, 0.42f)); // Subtle moonlit glow
    windowMesh.draw();
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));
}

void LabEnvironment::drawBook(const Shader& shader, const Mat4& baseTransform, const Vec3& coverColor) const {
    // Embossed leather hardcover
    shader.setBool("uUseTexture", true);
    bookLeatherTex.bind(0);
    shader.setInt("uTexture", 0);
    shader.setVec3("uDiffuseColor", coverColor);
    shader.setVec3("uSpecularColor", Vec3(0.28f, 0.22f, 0.14f));
    shader.setFloat("uShininess", 22.0f);
    setModelAndNormal(shader, baseTransform);
    bookCoverMesh.draw();

    // Cream page block slightly inset on right/front sides
    shader.setBool("uUseTexture", false);
    shader.setVec3("uDiffuseColor", Vec3(0.86f, 0.80f, 0.66f));
    shader.setVec3("uSpecularColor", Vec3(0.05f, 0.05f, 0.05f));
    shader.setFloat("uShininess", 6.0f);
    setModelAndNormal(shader, baseTransform * Mat4::translate(0.012f, 0.0f, 0.012f));
    bookPagesMesh.draw();
}

void LabEnvironment::renderLabProps(const Shader& standardShader, const Vec3& potionGlowColor, float resonance) const {
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // --- 1. Stacked Alchemy Spellbooks on Left Foreground ---
    Mat4 bookBase = Mat4::translate(-0.98f, 0.028f, 0.30f) * Mat4::rotateY(0.22f);
    drawBook(standardShader, bookBase, Vec3(0.62f, 0.14f, 0.16f)); // Deep Crimson Tome
    drawBook(standardShader, bookBase * Mat4::translate(0.02f, 0.056f, -0.01f) * Mat4::rotateY(-0.34f),
             Vec3(0.14f, 0.24f, 0.52f)); // Midnight Navy Grimoire
    drawBook(standardShader, bookBase * Mat4::translate(-0.01f, 0.112f, 0.01f) * Mat4::rotateY(0.18f),
             Vec3(0.14f, 0.42f, 0.24f)); // Emerald Herbalist Codex

    // --- 2. Stone Mortar & Pestle on Right Foreground ---
    standardShader.setBool("uUseTexture", true);
    mortarTex.bind(0);
    standardShader.setVec3("uDiffuseColor", Vec3(0.75f, 0.72f, 0.68f));
    standardShader.setVec3("uSpecularColor", Vec3(0.14f, 0.14f, 0.14f));
    standardShader.setFloat("uShininess", 12.0f);

    Mat4 mortarModel = Mat4::translate(1.06f, 0.0f, 0.26f);
    setModelAndNormal(standardShader, mortarModel);
    mortarBowlMesh.draw();

    // Pestle resting diagonally inside mortar (parent-child child of mortar!)
    Mat4 pestleModel = mortarModel * Mat4::translate(0.02f, 0.05f, 0.0f) * Mat4::rotateZ(-0.42f);
    setModelAndNormal(standardShader, pestleModel);
    pestleMesh.draw();

    // --- 3. Back Wall Wooden Apothecary Shelf + Decorative Reagent Jars ---
    woodTex.bind(0);
    standardShader.setVec3("uDiffuseColor", Vec3(0.85f, 0.78f, 0.70f));
    standardShader.setVec3("uSpecularColor", Vec3(0.14f, 0.10f, 0.08f));
    standardShader.setFloat("uShininess", 16.0f);

    Mat4 shelfModel = Mat4::translate(-0.78f, 1.15f, -2.18f);
    setModelAndNormal(standardShader, shelfModel);
    shelfBoardMesh.draw();
    setModelAndNormal(standardShader, shelfModel * Mat4::translate(-0.55f, -0.13f, 0.0f));
    shelfBracketMesh.draw();
    setModelAndNormal(standardShader, shelfModel * Mat4::translate( 0.55f, -0.13f, 0.0f));
    shelfBracketMesh.draw();

    // Colored apothecary jars resting on shelf (parent-child of shelfModel!)
    standardShader.setBool("uUseTexture", false);
    const float jarX[4] = { -0.52f, -0.18f, 0.18f, 0.50f };
    const Vec3 jarCols[4] = {
        Vec3(0.85f, 0.22f, 0.18f),
        Vec3(0.20f, 0.78f, 0.45f),
        Vec3(0.55f, 0.28f, 0.88f),
        Vec3(0.92f, 0.70f, 0.16f)
    };
    for (int i = 0; i < 4; ++i) {
        Mat4 mJar = shelfModel * Mat4::translate(jarX[i], 0.025f, 0.02f);
        setModelAndNormal(standardShader, mJar);
        standardShader.setVec3("uDiffuseColor", jarCols[i]);
        standardShader.setVec3("uSpecularColor", Vec3(0.75f, 0.75f, 0.80f));
        standardShader.setFloat("uShininess", 48.0f);
        standardShader.setVec3("uEmissive", jarCols[i] * 0.12f);
        jarMesh.draw();
    }
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // --- 4. Hanging Brass Alchemical Astrolabe (3-Level Parent-Child Hierarchy!) ---
    // Level 0: Ceiling Mount Rod (extends down into upper viewport above the workbench)
    Mat4 mMount = Mat4::translate(0.0f, 1.04f, -0.18f);
    standardShader.setVec3("uDiffuseColor", Vec3(0.76f, 0.60f, 0.25f));
    standardShader.setVec3("uSpecularColor", Vec3(0.98f, 0.88f, 0.54f));
    standardShader.setFloat("uShininess", 64.0f);
    setModelAndNormal(standardShader, mMount);
    astrolabeRodMesh.draw();

    // Level 1: Outer Brass Ring (child of Mount, rotates around Y)
    Mat4 mOuter = mMount * Mat4::translate(0.0f, -0.02f, 0.0f) * Mat4::rotateY(astrolabeAngle);
    setModelAndNormal(standardShader, mOuter);
    astrolabeOuterRing.draw();

    // Level 2: Inner Brass Ring (child of Outer Ring, rotates around X & tilted Z)
    Mat4 mInner = mOuter * Mat4::rotateX(astrolabeAngle * 1.45f) * Mat4::rotateZ(0.45f);
    setModelAndNormal(standardShader, mInner);
    astrolabeInnerRing.draw();

    // Level 3: Arcane Crystal Core (child of Inner Ring, spins & glows with potion resonance!)
    Mat4 mCrystal = mInner * Mat4::rotateY(astrolabeAngle * 2.2f) * Mat4::scale(0.85f, 1.35f, 0.85f);
    setModelAndNormal(standardShader, mCrystal);
    standardShader.setVec3("uDiffuseColor", potionGlowColor);
    standardShader.setVec3("uSpecularColor", Vec3(1.0f, 1.0f, 1.0f));
    standardShader.setFloat("uShininess", 96.0f);
    standardShader.setVec3("uEmissive", potionGlowColor * (0.55f + resonance * 1.15f));
    astrolabeCrystal.draw();
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));
}

void LabEnvironment::renderCandles(const Shader& standardShader) const {
    standardShader.setBool("uUseTexture", false);
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));

    // Left brass saucer
    setModelAndNormal(standardShader, Mat4::translate(-0.88f, 0.0f, -0.18f));
    standardShader.setVec3("uDiffuseColor", Vec3(0.72f, 0.58f, 0.26f));
    standardShader.setVec3("uSpecularColor", Vec3(0.95f, 0.85f, 0.55f));
    standardShader.setFloat("uShininess", 48.0f);
    candleSaucerLeftMesh.draw();

    // Right brass saucer
    setModelAndNormal(standardShader, Mat4::translate(0.91f, 0.0f, -0.23f));
    standardShader.setVec3("uDiffuseColor", Vec3(0.65f, 0.54f, 0.28f));
    standardShader.setVec3("uSpecularColor", Vec3(0.90f, 0.80f, 0.50f));
    standardShader.setFloat("uShininess", 40.0f);
    candleSaucerRightMesh.draw();

    // Candle wax cylinders & charred wicks
    for (const auto& c : candles) {
        Mat4 mWax = Mat4::translate(c.position) * Mat4::scale(c.radius, c.height, c.radius);
        setModelAndNormal(standardShader, mWax);
        standardShader.setVec3("uDiffuseColor", Vec3(0.92f, 0.88f, 0.77f));
        standardShader.setVec3("uSpecularColor", Vec3(0.20f, 0.18f, 0.15f));
        standardShader.setFloat("uShininess", 12.0f);
        candleWaxUnitMesh.draw();

        Mat4 mWick = Mat4::translate(c.position + Vec3(0.0f, c.height, 0.0f));
        setModelAndNormal(standardShader, mWick);
        standardShader.setVec3("uDiffuseColor", Vec3(0.10f, 0.08f, 0.06f));
        standardShader.setVec3("uSpecularColor", Vec3(0.0f, 0.0f, 0.0f));
        standardShader.setFloat("uShininess", 1.0f);
        candleWickMesh.draw();
    }

    // Melted wax beads around saucers
    standardShader.setVec3("uDiffuseColor", Vec3(0.90f, 0.86f, 0.75f));
    standardShader.setVec3("uSpecularColor", Vec3(0.18f, 0.16f, 0.14f));
    standardShader.setFloat("uShininess", 10.0f);
    const Vec3 drips[5] = {
        Vec3(-0.84f, 0.019f, -0.14f), Vec3(-0.92f, 0.019f, -0.22f),
        Vec3(-0.75f, 0.019f, -0.07f), Vec3( 0.84f, 0.017f, -0.16f),
        Vec3( 0.93f, 0.017f, -0.25f)
    };
    for (int i = 0; i < 5; ++i) {
        setModelAndNormal(standardShader, Mat4::translate(drips[i]) * Mat4::scale(1.2f, 0.35f, 1.2f));
        waxDropMesh.draw();
    }
}

void LabEnvironment::renderParchment(const Shader& standardShader) const {
    standardShader.setBool("uUseTexture", true);
    parchmentTex.bind(0);
    standardShader.setInt("uTexture", 0);

    Mat4 m = Mat4::translate(0.58f, 0.002f, 0.25f) * Mat4::rotateY(-0.26f);
    setModelAndNormal(standardShader, m);
    standardShader.setVec3("uDiffuseColor", Vec3(1.0f, 0.98f, 0.92f));
    standardShader.setVec3("uSpecularColor", Vec3(0.04f, 0.04f, 0.04f));
    standardShader.setFloat("uShininess", 4.0f);
    standardShader.setVec3("uEmissive", Vec3(0.0f, 0.0f, 0.0f));
    parchmentPlaneMesh.draw();
}

void LabEnvironment::renderFlamesAndGlow(const Shader& flameShader, const Shader& glowShader,
                                         const Mat4& view, const Mat4& proj, float timeElapsed,
                                         bool candlesLit, bool isFrost, float flicker0, float flicker1,
                                         const Vec3& flaskPos, const Vec3& potionColor, float resonance) const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    Vec3 flameTint = isFrost ? Vec3(0.55f, 0.92f, 1.25f) : Vec3(1.0f, 1.0f, 1.0f);

    Mat4 billboardRot = Mat4::identity();
    billboardRot.at(0, 0) = view.at(0, 0); billboardRot.at(1, 0) = view.at(0, 1); billboardRot.at(2, 0) = view.at(0, 2);
    billboardRot.at(0, 1) = view.at(1, 0); billboardRot.at(1, 1) = view.at(1, 1); billboardRot.at(2, 1) = view.at(1, 2);
    billboardRot.at(0, 2) = view.at(2, 0); billboardRot.at(1, 2) = view.at(2, 1); billboardRot.at(2, 2) = view.at(2, 2);

    if (candlesLit) {
        flameShader.use();
        flameShader.setMat4("uView", view);
        flameShader.setMat4("uProjection", proj);
        flameShader.setFloat("uTime", timeElapsed);
        flameShader.setVec3("uFlameTint", flameTint);

        for (const auto& c : candles) {
            Vec3 flamePos = c.position + Vec3(0.0f, c.height + 0.008f, 0.0f);
            flameShader.setMat4("uModel", Mat4::translate(flamePos) * Mat4::scale(c.flameScale));
            flameShader.setFloat("uSeed", c.seed);
            flameMesh.draw();
        }

        glowShader.use();
        glowShader.setMat4("uView", view);
        glowShader.setMat4("uProjection", proj);
        glowShader.setVec3("uGlowColor", isFrost ? Vec3(0.45f, 0.85f, 1.0f) : Vec3(1.0f, 0.68f, 0.22f));

        for (size_t i = 0; i < candles.size(); ++i) {
            const auto& c = candles[i];
            float flicker = (i < 3) ? flicker0 : flicker1;
            Vec3 haloPos = c.position + Vec3(0.0f, c.height + 0.055f, 0.0f);
            Mat4 mHalo = Mat4::translate(haloPos) * billboardRot * Mat4::scale(0.28f * c.flameScale);
            glowShader.setMat4("uModel", mHalo);
            glowShader.setFloat("uIntensity", 0.68f * flicker);
            billboardMesh.draw();
        }
    }

    // Potion ethereal aura (glows even when candles are OFF)
    glowShader.use();
    glowShader.setMat4("uView", view);
    glowShader.setMat4("uProjection", proj);
    glowShader.setVec3("uGlowColor", potionColor);
    float auraScale = 0.46f + resonance * 0.42f;
    float auraInt = 0.48f + resonance * 0.72f + 0.14f * std::sin(timeElapsed * 4.0f);
    glowShader.setMat4("uModel", Mat4::translate(flaskPos + Vec3(0.0f, 0.18f, 0.0f)) * billboardRot * Mat4::scale(auraScale));
    glowShader.setFloat("uIntensity", auraInt);
    billboardMesh.draw();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
