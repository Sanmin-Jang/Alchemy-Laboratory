#ifndef LAB_ENVIRONMENT_H
#define LAB_ENVIRONMENT_H

#include <vector>
#include "math3d.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

struct Candle {
    Vec3 position;
    float height;
    float radius;
    float seed;
    float flameScale;
};

class LabEnvironment {
public:
    std::vector<Candle> candles;
    float astrolabeAngle;

    LabEnvironment();
    ~LabEnvironment();

    void init();
    void update(float dt, float magicResonance);

    void renderRoomAndTable(const Shader& standardShader) const;
    void renderLabProps(const Shader& standardShader, const Vec3& potionGlowColor, float resonance) const;
    void renderCandles(const Shader& standardShader) const;
    void renderParchment(const Shader& standardShader) const;
    void renderFlamesAndGlow(const Shader& flameShader, const Shader& glowShader,
                             const Mat4& view, const Mat4& proj, float timeElapsed,
                             bool candlesLit, bool isFrost, float flicker0, float flicker1,
                             const Vec3& flaskPos, const Vec3& potionColor, float resonance) const;

    const Mesh& getFlameMesh() const { return flameMesh; }
    const Mesh& getBillboardMesh() const { return billboardMesh; }

private:
    // Procedural textures (all mipmapped)
    Texture woodTex;
    Texture stoneTex;
    Texture parchmentTex;
    Texture mortarTex;
    Texture bookLeatherTex;
    Texture windowTex;

    // Room & workbench meshes
    Mesh tableTopMesh;
    Mesh tableLegMesh;
    Mesh tableApronLongMesh;
    Mesh tableApronShortMesh;
    Mesh floorMesh;
    Mesh wallWideMesh;
    Mesh wallSideMesh;
    Mesh beamMesh;
    Mesh pillarMesh;
    Mesh windowMesh;
    Mesh parchmentPlaneMesh;

    // Books, shelf, mortar & astrolabe hierarchy meshes
    Mesh bookCoverMesh;
    Mesh bookPagesMesh;
    Mesh shelfBoardMesh;
    Mesh shelfBracketMesh;
    Mesh mortarBowlMesh;
    Mesh pestleMesh;
    Mesh jarMesh;
    Mesh astrolabeRodMesh;
    Mesh astrolabeOuterRing;
    Mesh astrolabeInnerRing;
    Mesh astrolabeCrystal;

    // Candle meshes
    Mesh candleWaxUnitMesh;
    Mesh candleSaucerLeftMesh;
    Mesh candleSaucerRightMesh;
    Mesh candleWickMesh;
    Mesh flameMesh;
    Mesh billboardMesh;
    Mesh waxDropMesh;

    void setModelAndNormal(const Shader& shader, const Mat4& model) const;
    void drawBook(const Shader& shader, const Mat4& baseTransform, const Vec3& coverColor) const;
};

#endif // LAB_ENVIRONMENT_H
