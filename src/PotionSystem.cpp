#include "PotionSystem.h"
#include <algorithm>
#include <cmath>

PotionSystem::PotionSystem()
    : slot1(ING_NONE),
      slot2(ING_NONE),
      activeEffect(EFFECT_NONE),
      effectTimer(0.0f),
      effectDuration(6.0f),
      corkOffset(0.0f, 0.0f, 0.0f),
      corkVelocity(0.0f, 0.0f, 0.0f),
      corkSpinAngle(0.0f),
      corkPopped(false),
      shockwaveRadius(0.0f),
      shockwaveAlpha(0.0f),
      shakeTimer(0.0f),
      shakeMagnitude(0.0f),
      currentPotionTitle("Empty Brewing Flask"),
      statusBanner("Add 2 ingredients [1][2][3] or click vials, then press [E] to Brew!"),
      bannerTimer(8.0f) {
    discovered[0] = discovered[1] = discovered[2] = false;
}

void PotionSystem::init() {
    bottles.clear();
    bottles.push_back({ ING_EMBER, "Crimson Ember", Vec3(-0.48f, 0.0f, -0.18f), Vec3(0.98f, 0.16f, 0.14f), 0.0f });
    bottles.push_back({ ING_FROST, "Frost Dew",     Vec3(-0.24f, 0.0f, -0.50f), Vec3(0.14f, 0.62f, 1.00f), 0.0f });
    bottles.push_back({ ING_VENOM, "Venom Leaf",    Vec3( 0.45f, 0.0f, -0.24f), Vec3(0.14f, 0.96f, 0.36f), 0.0f });
}

std::string PotionSystem::getIngredientName(IngredientType t) {
    switch (t) {
        case ING_EMBER: return "Crimson Ember";
        case ING_FROST: return "Frost Dew";
        case ING_VENOM: return "Venom Leaf";
        default:        return "Empty";
    }
}

Vec3 PotionSystem::getIngredientColor(IngredientType t) {
    switch (t) {
        case ING_EMBER: return Vec3(0.98f, 0.16f, 0.14f);
        case ING_FROST: return Vec3(0.14f, 0.62f, 1.00f);
        case ING_VENOM: return Vec3(0.14f, 0.96f, 0.36f);
        default:        return Vec3(0.35f, 0.55f, 0.65f);
    }
}

bool PotionSystem::addIngredient(IngredientType type, Vec3& outMixedColor, float& outNewFill) {
    if (type == ING_NONE) return false;

    // If a reaction just finished or both slots are already full, start a fresh brew automatically
    if (slot1 != ING_NONE && slot2 != ING_NONE) {
        clearFlask();
    }

    // Trigger bottle pouring animation
    for (auto& b : bottles) {
        if (b.type == type) {
            b.pourTimer = 1.0f;
            break;
        }
    }

    if (slot1 == ING_NONE) {
        slot1 = type;
        outMixedColor = getIngredientColor(slot1);
        outNewFill = 0.42f;
        currentPotionTitle = getIngredientName(slot1) + " Base";
        statusBanner = "Added " + getIngredientName(slot1) + " (1/2). Add a second ingredient!";
        bannerTimer = 4.5f;
        return true;
    }

    slot2 = type;
    outMixedColor = (getIngredientColor(slot1) + getIngredientColor(slot2)) * 0.5f;
    // Boost saturation slightly so intermediate mixture looks vibrant
    float maxC = std::max(outMixedColor.x, std::max(outMixedColor.y, outMixedColor.z));
    if (maxC > 0.01f) outMixedColor = outMixedColor * (0.95f / maxC);
    outNewFill = 0.78f;
    currentPotionTitle = "Unbrewed Mixture (Ready!)";
    statusBanner = "Mixture ready! Press [E] or [ENTER] to BREW!";
    bannerTimer = 6.0f;
    return true;
}

bool PotionSystem::canBrew() const {
    return (slot1 != ING_NONE && slot2 != ING_NONE);
}

PotionEffect PotionSystem::brew(Vec3& outResultColor) {
    if (!canBrew()) return EFFECT_NONE;

    int a = (int)slot1;
    int b = (int)slot2;
    if (a > b) std::swap(a, b);

    // Reset explosion cork state before new brew
    corkPopped = false;
    corkOffset = Vec3(0.0f, 0.0f, 0.0f);
    corkVelocity = Vec3(0.0f, 0.0f, 0.0f);
    corkSpinAngle = 0.0f;
    shockwaveRadius = 0.0f;
    shockwaveAlpha = 0.0f;

    if (a == ING_EMBER && b == ING_VENOM) {
        // Combination 1: Crimson Ember + Venom Leaf -> VOLATILE INFERNO (EXPLOSION!)
        activeEffect = EFFECT_EXPLOSION;
        effectDuration = 5.5f;
        effectTimer = effectDuration;
        outResultColor = Vec3(1.0f, 0.36f, 0.06f);
        currentPotionTitle = "Volatile Inferno (EXPLOSION!)";
        statusBanner = "BOOM! Volatile Inferno detonated! Cork launched!";
        bannerTimer = 5.5f;

        corkPopped = true;
        corkVelocity = Vec3(0.35f, 3.8f, 0.25f);
        shockwaveRadius = 0.12f;
        shockwaveAlpha = 1.0f;
        shakeTimer = 0.70f;
        shakeMagnitude = 0.075f;
        discovered[0] = true;
    } else if (a == ING_EMBER && b == ING_FROST) {
        // Combination 2: Crimson Ember + Frost Dew -> ASTRAL STARLIGHT (ARCANE NOVA!)
        activeEffect = EFFECT_ARCANE_NOVA;
        effectDuration = 7.5f;
        effectTimer = effectDuration;
        outResultColor = Vec3(0.86f, 0.24f, 1.00f);
        currentPotionTitle = "Astral Starlight (ARCANE NOVA!)";
        statusBanner = "ARCANE TRANSMUTATION! The finished potion illuminates the dark lab!";
        bannerTimer = 6.5f;
        discovered[1] = true;
    } else if (a == ING_FROST && b == ING_VENOM) {
        // Combination 3: Frost Dew + Venom Leaf -> GLACIAL MIST (CRYO FREEZE!)
        activeEffect = EFFECT_GLACIAL_MIST;
        effectDuration = 7.0f;
        effectTimer = effectDuration;
        outResultColor = Vec3(0.14f, 0.94f, 1.00f);
        currentPotionTitle = "Glacial Mist (CRYO FREEZE!)";
        statusBanner = "CRYO FREEZE! Swirling glacial mist cools the laboratory!";
        bannerTimer = 6.0f;
        discovered[2] = true;
    } else {
        // Same ingredient twice -> Pure Essence
        activeEffect = EFFECT_PURE_ESSENCE;
        effectDuration = 3.5f;
        effectTimer = effectDuration;
        outResultColor = getIngredientColor(slot1);
        currentPotionTitle = "Pure " + getIngredientName(slot1) + " Essence";
        statusBanner = "Pure essence brewed! Mix 2 DIFFERENT ingredients for magical reactions!";
        bannerTimer = 5.0f;
    }

    if (getDiscoveredCount() == 3 && activeEffect != EFFECT_PURE_ESSENCE) {
        statusBanner += " [ALL 3 RECIPES MASTERED!]";
    }
    return activeEffect;
}

void PotionSystem::clearFlask() {
    slot1 = ING_NONE;
    slot2 = ING_NONE;
    activeEffect = EFFECT_NONE;
    effectTimer = 0.0f;
    corkPopped = false;
    corkOffset = Vec3(0.0f, 0.0f, 0.0f);
    corkVelocity = Vec3(0.0f, 0.0f, 0.0f);
    corkSpinAngle = 0.0f;
    shockwaveRadius = 0.0f;
    shockwaveAlpha = 0.0f;
    currentPotionTitle = "Empty Brewing Flask";
    statusBanner = "Flask cleared. Add 2 ingredients [1][2][3] or click vials to mix!";
    bannerTimer = 4.0f;
}

void PotionSystem::resetProgress() {
    discovered[0] = discovered[1] = discovered[2] = false;
    clearFlask();
}

void PotionSystem::update(float dt) {
    if (bannerTimer > 0.0f) {
        bannerTimer = std::max(0.0f, bannerTimer - dt);
    }

    // Update bottle pouring animations
    for (auto& b : bottles) {
        if (b.pourTimer > 0.0f) {
            b.pourTimer = std::max(0.0f, b.pourTimer - dt * 1.45f);
        }
    }

    // Update active reaction timer
    if (effectTimer > 0.0f) {
        effectTimer = std::max(0.0f, effectTimer - dt);
        if (effectTimer <= 0.0f && activeEffect == EFFECT_PURE_ESSENCE) {
            activeEffect = EFFECT_NONE;
        }
    }

    // Update launched cork physics during explosion
    if (corkPopped) {
        corkVelocity.y -= 7.8f * dt;
        corkOffset += corkVelocity * dt;
        corkSpinAngle += dt * 14.0f;
        // Stop on tabletop level
        if (corkOffset.y < -0.42f) {
            corkOffset.y = -0.42f;
            corkVelocity.y *= -0.35f;
            corkVelocity.x *= 0.65f;
            corkVelocity.z *= 0.65f;
        }
    }

    // Update expanding shockwave ring
    if (shockwaveAlpha > 0.0f) {
        shockwaveRadius += dt * 2.2f;
        shockwaveAlpha = std::max(0.0f, shockwaveAlpha - dt * 1.45f);
    }

    // Update camera shake timer
    if (shakeTimer > 0.0f) {
        shakeTimer = std::max(0.0f, shakeTimer - dt);
    }
}

IngredientType PotionSystem::hitTestBottle(const Vec3& rayOrigin, const Vec3& rayDir) const {
    float closestT = 1e9f;
    IngredientType hit = ING_NONE;

    for (const auto& b : bottles) {
        Vec3 center = b.basePos + Vec3(0.0f, 0.14f, 0.0f);
        Vec3 oc = rayOrigin - center;
        float bDot = Vec3::dot(oc, rayDir);
        float cVal = Vec3::dot(oc, oc) - 0.15f * 0.15f;
        float disc = bDot * bDot - cVal;
        if (disc > 0.0f) {
            float t = -bDot - std::sqrt(disc);
            if (t > 0.0f && t < closestT) {
                closestT = t;
                hit = b.type;
            }
        }
    }
    return hit;
}

Mat4 PotionSystem::getBottleModelMatrix(size_t index, const Vec3& mainFlaskPos) const {
    const auto& b = bottles[index];
    if (b.pourTimer <= 0.001f) {
        return Mat4::translate(b.basePos);
    }

    // Smooth arch from basePos toward mainFlaskPos mouth and back
    float arc = std::sin(b.pourTimer * (float)M_PI);
    Vec3 pourTarget = mainFlaskPos + Vec3((b.basePos.x > 0.0f ? 0.14f : -0.14f), 0.56f, 0.0f);
    Vec3 pos = Vec3::lerp(b.basePos, pourTarget, arc);
    float tiltAngle = (b.basePos.x >= 0.0f ? 1.15f : -1.15f) * arc;

    return Mat4::translate(pos) * Mat4::rotateZ(tiltAngle);
}

int PotionSystem::getDiscoveredCount() const {
    return (discovered[0] ? 1 : 0) + (discovered[1] ? 1 : 0) + (discovered[2] ? 1 : 0);
}
