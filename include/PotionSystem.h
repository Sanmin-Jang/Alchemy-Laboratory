#ifndef POTION_SYSTEM_H
#define POTION_SYSTEM_H

#include <string>
#include <vector>
#include "math3d.h"

enum IngredientType {
    ING_NONE = 0,
    ING_EMBER = 1, // [1] Crimson Ember (Red)
    ING_FROST = 2, // [2] Frost Dew (Blue)
    ING_VENOM = 3  // [3] Venom Leaf (Green)
};

enum PotionEffect {
    EFFECT_NONE = 0,
    EFFECT_EXPLOSION = 1,    // Ember + Venom -> Volatile Inferno (Explosion!)
    EFFECT_ARCANE_NOVA = 2,  // Ember + Frost -> Astral Starlight (Room-lighting Wow Moment!)
    EFFECT_GLACIAL_MIST = 3, // Frost + Venom -> Glacial Mist (Cryo-Freeze!)
    EFFECT_PURE_ESSENCE = 4  // Same ingredient x2 -> Concentrated Pure Essence
};

struct IngredientBottle {
    IngredientType type;
    std::string name;
    Vec3 basePos;
    Vec3 color;
    float pourTimer; // > 0 while animating lift & pour toward main flask
};

class PotionSystem {
public:
    std::vector<IngredientBottle> bottles;
    IngredientType slot1;
    IngredientType slot2;

    PotionEffect activeEffect;
    float effectTimer;
    float effectDuration;

    // Explosion cork projectile & shockwave state
    Vec3 corkOffset;
    Vec3 corkVelocity;
    float corkSpinAngle;
    bool corkPopped;
    float shockwaveRadius;
    float shockwaveAlpha;

    // Camera shake state (driven by explosion)
    float shakeTimer;
    float shakeMagnitude;

    // Feedback & progression tracking
    std::string currentPotionTitle;
    std::string statusBanner;
    float bannerTimer;
    bool discovered[3]; // 0 = Explosion, 1 = Arcane Nova, 2 = Glacial Mist

    PotionSystem();

    void init();
    void update(float dt);

    // Add ingredient (1, 2, or 3) into slot1 or slot2; returns true if accepted
    bool addIngredient(IngredientType type, Vec3& outMixedColor, float& outNewFill);

    // Brew the 2 ingredients into one of the 3 potion combinations
    bool canBrew() const;
    PotionEffect brew(Vec3& outResultColor);

    // Reset mixing slots and active effects (keeps discovery count!)
    void clearFlask();
    void resetProgress();

    // Ray-sphere picking for the 3 ingredient bottles on the workbench
    IngredientType hitTestBottle(const Vec3& rayOrigin, const Vec3& rayDir) const;

    // Parent-child world transform for an animated ingredient bottle
    Mat4 getBottleModelMatrix(size_t index, const Vec3& mainFlaskPos) const;

    int getDiscoveredCount() const;
    static std::string getIngredientName(IngredientType t);
    static Vec3 getIngredientColor(IngredientType t);
};

#endif // POTION_SYSTEM_H
