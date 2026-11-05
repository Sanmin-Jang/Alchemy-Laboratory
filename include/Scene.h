#ifndef SCENE_H
#define SCENE_H

#include <string>
#include "math3d.h"
#include "Shader.h"
#include "Mesh.h"
#include "SloshPhysics.h"

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

    // Flask picking ray intersection
    bool hitTestFlask(const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) const;

    // Helper: calculate flask radius at a given height
    static float getFlaskRadiusAtHeight(float h);

private:
    // Shaders for glassware, dynamic sloshing liquid, and cork
    Shader glassShader;
    Shader liquidShader;
    Shader corkShader;

    // Geometry meshes
    Mesh flaskMesh;
    Mesh liquidMesh;
    Mesh diskMesh;
    Mesh corkMesh;

    // Render pipeline passes
    void renderLiquid(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderGlass(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderCork(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
};

#endif // SCENE_H
