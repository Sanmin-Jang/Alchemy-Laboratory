#ifndef SCENE_H
#define SCENE_H

#include <string>
#include "math3d.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "SloshPhysics.h"
#include "LabEnvironment.h"
#include "PotionSystem.h"
#include "ParticleSystem.h"

class Scene {
public:
    // Core modules
    LabEnvironment env;
    PotionSystem potions;
    SloshPhysics slosh;
    ParticleSystem particles;

    // Interactive main flask state
    Vec3 flaskPos;
    bool isDragging;
    float timeElapsed;
    float runeRotation;

    // Potion color interpolation
    Vec4 targetLiquidColor;

    // Atmospheric candle lighting state
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
    void toggleCandles();

    // Potion mixing actions
    bool addIngredient(IngredientType type);
    PotionEffect brewPotion();

    // Ray picking
    bool hitTestFlask(const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) const;
    IngredientType hitTestIngredient(const Vec3& rayOrigin, const Vec3& rayDir) const;

    // Camera shake offset for Explosion effect
    Vec3 getCameraShakeOffset() const;

    // Dynamic clear / fog color for atmospheric immersion
    Vec3 getBackgroundFogColor() const;

private:
    // Shaders
    Shader glassShader;
    Shader liquidShader;
    Shader corkShader;
    Shader standardShader;
    Shader flameShader;
    Shader runeShader;
    Shader glowShader;

    // Textures
    Texture runeTex;

    // Main flask & ingredient bottle meshes
    Mesh flaskMesh;
    Mesh liquidMesh;
    Mesh diskMesh;
    Mesh corkMesh;
    Mesh runePlaneMesh;
    Mesh shockwaveMesh;

    Mesh smallBottleGlassMesh;
    Mesh smallBottleLiquidMesh;
    Mesh smallBottleCorkMesh;

    // Continuous particle spawn timers
    float bubbleTimer;
    float steamTimer;
    float effectParticleTimer;

    // Render pipeline passes
    void setupStandardLights(const Shader& shader, const Vec3& cameraPos) const;
    void setupCandleLights(const Shader& shader) const;
    void renderIngredientBottles(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderCork(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderRuneCircle(const Mat4& view, const Mat4& proj);
    void renderLiquid(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderGlass(const Mat4& view, const Mat4& proj, const Vec3& cameraPos);
    void renderShockwave(const Mat4& view, const Mat4& proj);

    float getMagicResonance() const;
};

#endif // SCENE_H
