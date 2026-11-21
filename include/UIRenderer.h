#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <GL/glew.h>
#include <string>
#include <vector>
#include "math3d.h"
#include "Shader.h"
#include "Texture.h"
#include "PotionSystem.h"
#include "Camera.h"

enum GameState {
    STATE_MAIN_MENU = 0,
    STATE_PLAYING   = 1,
    STATE_PAUSED    = 2,
    STATE_RECIPES   = 3
};

struct UIButton {
    float x, y, w, h;
    std::string label;
    int actionId;

    bool contains(float mx, float my) const {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }
};

struct HUDState {
    float fps;
    CameraMode camMode;
    bool candlesLit;
    float fillLevel;
    size_t particleCount;
    bool freeLook;
    const PotionSystem* potions;
};

class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();

    UIRenderer(const UIRenderer&) = delete;
    UIRenderer& operator=(const UIRenderer&) = delete;

    void init();

    // Render appropriate overlay for current GameState
    void render(GameState state, int screenW, int screenH,
                int mouseX, int mouseY, const HUDState& hud);

    // Hit-test active menu buttons; returns actionId or -1
    int hitTestButton(float mouseX, float mouseY) const;

private:
    Shader uiShader;
    Texture fontTex;
    GLuint vao;
    GLuint vbo;

    int curW;
    int curH;
    std::vector<UIButton> activeButtons;

    void begin2D(int screenW, int screenH);
    void end2D();

    void drawRect(float x, float y, float w, float h, const Vec4& color);
    void drawPanel(float x, float y, float w, float h, const Vec4& bg, const Vec4& border, float borderPx = 2.0f);
    void drawText(const std::string& text, float x, float y, float scale, const Vec4& color);
    void drawTextCentered(const std::string& text, float cx, float y, float scale, const Vec4& color);
    void drawButton(const UIButton& btn, int mouseX, int mouseY);

    void renderMainMenu(int mouseX, int mouseY, const HUDState& hud);
    void renderHUD(const HUDState& hud);
    void renderPauseMenu(int mouseX, int mouseY);
    void renderRecipesOverlay(int mouseX, int mouseY, const HUDState& hud);
};

#endif // UI_RENDERER_H
