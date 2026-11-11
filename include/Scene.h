#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "math3d.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "SloshPhysics.h"

// Wax candle structure with individual position, sizing, and flame animation parameters
struct Candle {
    Vec3 position;    // Base position on table
    float height;     // Height of wax cylinder
    float radius;     // Radius of wax cylinder
    float seed;       // Time offset for individual flame flicker
    float flameScale; // Relative size of flame
};

class Scene {
public:
    // Core physics and interactive state
    SloshPhysics slosh;
    Vec3 flaskPos;
    bool isDragging;
    float timeElapsed;

    // Potion metadata & appearance
    std::string potionName;
    std::string colorName;
    int currentColorIndex;
    Vec4 targetLiquidColor;

    // Atmospheric lighting state
    bool candlesLit;
    float candleFlicker0;
    float candleFlicker1;

    Scene();
    ~Scene();

    void init();
    void update(float dt);
    void render(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);

    // Interaction handlers
    void onFlaskDragStart();
    void onFlaskDrag(const Vec3& newPos);
    void onFlaskDragEnd();
    void adjustLiquidLevel(float delta);
    void stirPotion();
    void resetPotion();

    // Color controls
    void setColorPreset(int preset);
    void cycleColor();

    // Atmospheric candle controls
    void toggleCandles();

    // Flask picking ray intersection
    bool hitTestFlask(const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) const;

    // Helper: calculate flask radius at a given height
    static float getFlaskRadiusAtHeight(float h);

private:
    // Shaders
    Shader glassShader;
    Shader liquidShader;
    Shader corkShader;
    Shader standardShader;
    Shader flameShader;
    Shader runeShader;
    Shader glowShader;

    // Procedural textures
    Texture woodTex;
    Texture stoneTex;
    Texture parchmentTex;
    Texture runeTex;

    // Glassware & liquid meshes
    Mesh flaskMesh;
    Mesh liquidMesh;
    Mesh diskMesh;
    Mesh corkMesh;

    // Table & environment meshes
    Mesh tableTopMesh;
    Mesh tableLegMesh;
    Mesh tableApronLongMesh;
    Mesh tableApronShortMesh;
    Mesh floorMesh;
    Mesh backWallMesh;
    Mesh runePlaneMesh;
    Mesh parchmentPlaneMesh;

    // Candle meshes
    Mesh candleWaxUnitMesh;
    Mesh candleSaucerLeftMesh;
    Mesh candleSaucerRightMesh;
    Mesh candleWickMesh;
    Mesh flameMesh;
    Mesh billboardMesh;
    Mesh waxDropMesh;

    // Candle instances
    std::vector<Candle> candles;

    // Render pipeline passes
    void renderRoomAndTable(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderCandles(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderParchment(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderRuneCircle(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderLiquid(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderGlass(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderCork(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderFlamesAndGlow(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);

    // Light helper
    void setupLights(const Shader& shader, const Vec3& cameraPos) const;
};

#endif // SCENE_H
