#include "UIRenderer.h"
#include "BitmapFont.h"
#include <algorithm>
#include <cctype>
#include <sstream>

UIRenderer::UIRenderer() : vao(0), vbo(0), curW(1280), curH(720) {}

UIRenderer::~UIRenderer() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

void UIRenderer::init() {
    uiShader.load("shaders/ui.vert", "shaders/ui.frag");

    // Build 128x32 RGBA font texture atlas (16x4 grid of 8x8 glyph cells for ASCII 32..95)
    const int atlasW = 128;
    const int atlasH = 32;
    std::vector<unsigned char> pixels(atlasW * atlasH * 4, 0);

    for (int ch = 0; ch < 64; ++ch) {
        int col = ch % 16;
        int row = ch / 16;
        for (int py = 0; py < 7; ++py) {
            uint8_t bits = FONT_5X7[ch][py];
            for (int px = 0; px < 5; ++px) {
                if (bits & (1 << (4 - px))) {
                    int ax = col * 8 + px + 1;
                    int ay = row * 8 + py;
                    int idx = (ay * atlasW + ax) * 4;
                    pixels[idx + 0] = 255;
                    pixels[idx + 1] = 255;
                    pixels[idx + 2] = 255;
                    pixels[idx + 3] = 255;
                }
            }
        }
    }
    fontTex = Texture::createFromRGBA(atlasW, atlasH, pixels, true);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void UIRenderer::begin2D(int screenW, int screenH) {
    curW = std::max(1, screenW);
    curH = std::max(1, screenH);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uiShader.use();
    Mat4 ortho = Mat4::ortho(0.0f, (float)curW, (float)curH, 0.0f, -1.0f, 1.0f);
    uiShader.setMat4("uOrtho", ortho);
}

void UIRenderer::end2D() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void UIRenderer::drawRect(float x, float y, float w, float h, const Vec4& color) {
    float verts[24] = {
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f,
        x,     y,     0.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f,
        x,     y + h, 0.0f, 1.0f
    };
    uiShader.setBool("uUseTexture", false);
    uiShader.setVec4("uColor", color);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void UIRenderer::drawPanel(float x, float y, float w, float h, const Vec4& bg, const Vec4& border, float b) {
    drawRect(x - b, y - b, w + 2.0f * b, h + 2.0f * b, border);
    drawRect(x, y, w, h, bg);
}

void UIRenderer::drawText(const std::string& text, float x, float y, float scale, const Vec4& color) {
    if (text.empty()) return;

    std::vector<float> verts;
    verts.reserve(text.size() * 24);

    float charW = std::floor(6.0f * scale + 0.5f);
    float glyphW = std::floor(8.0f * scale + 0.5f);
    float glyphH = std::floor(7.0f * scale + 0.5f);
    float cursorX = std::floor(x + 0.5f);
    float baseY = std::floor(y + 0.5f);

    const float atlasW = 128.0f;
    const float atlasH = 32.0f;
    const float epsU = 0.20f / atlasW;
    const float epsV = 0.20f / atlasH;

    for (char rawC : text) {
        int c = (unsigned char)std::toupper((unsigned char)rawC);
        if (c <= 32 || c > 95) {
            cursorX += charW;
            continue;
        }
        int idx = c - 32;
        int col = idx % 16;
        int row = idx / 16;

        // Inset UVs within the 7-row glyph area so adjacent atlas rows never bleed in fullscreen
        float u0 = (col * 8.0f) / atlasW + epsU;
        float u1 = ((col + 1) * 8.0f) / atlasW - epsU;
        float v0 = (row * 8.0f) / atlasH + epsV;
        float v1 = (row * 8.0f + 7.0f) / atlasH - epsV;

        float x0 = cursorX;
        float y0 = baseY;
        float x1 = cursorX + glyphW;
        float y1 = baseY + glyphH;

        float q[24] = {
            x0, y0, u0, v0,  x1, y0, u1, v0,  x1, y1, u1, v1,
            x0, y0, u0, v0,  x1, y1, u1, v1,  x0, y1, u0, v1
        };
        verts.insert(verts.end(), q, q + 24);
        cursorX += charW;
        if (verts.size() >= 256 * 24) break;
    }

    if (verts.empty()) return;

    uiShader.setBool("uUseTexture", true);
    uiShader.setVec4("uColor", color);
    fontTex.bind(0);
    uiShader.setInt("uFontTex", 0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 4));
    glBindVertexArray(0);
}

void UIRenderer::drawTextCentered(const std::string& text, float cx, float y, float scale, const Vec4& color) {
    float totalW = (float)text.size() * 6.0f * scale;
    drawText(text, cx - totalW * 0.5f, y, scale, color);
}

void UIRenderer::drawButton(const UIButton& btn, int mouseX, int mouseY) {
    bool hovered = btn.contains((float)mouseX, (float)mouseY);
    Vec4 bg     = hovered ? Vec4(0.28f, 0.21f, 0.11f, 0.94f) : Vec4(0.11f, 0.10f, 0.14f, 0.88f);
    Vec4 border = hovered ? Vec4(0.98f, 0.80f, 0.34f, 1.0f)  : Vec4(0.62f, 0.48f, 0.24f, 0.90f);
    Vec4 txtCol = hovered ? Vec4(1.0f, 0.96f, 0.78f, 1.0f)   : Vec4(0.90f, 0.86f, 0.78f, 1.0f);
    drawPanel(btn.x, btn.y, btn.w, btn.h, bg, border, 2.0f);
    drawTextCentered(btn.label, btn.x + btn.w * 0.5f, btn.y + (btn.h - 16.0f) * 0.5f, 2.0f, txtCol);
}

int UIRenderer::hitTestButton(float mouseX, float mouseY) const {
    for (const auto& b : activeButtons) {
        if (b.contains(mouseX, mouseY)) return b.actionId;
    }
    return -1;
}

void UIRenderer::render(GameState state, int screenW, int screenH,
                        int mouseX, int mouseY, const HUDState& hud) {
    activeButtons.clear();
    begin2D(screenW, screenH);

    // Always draw crisp FPS badge in top-left corner (R7 & Demo Video G3 requirement!)
    std::string fpsText = "FPS: " + std::to_string((int)(hud.fps + 0.5f));
    drawPanel(14.0f, 14.0f, 118.0f, 30.0f, Vec4(0.06f, 0.06f, 0.09f, 0.80f), Vec4(0.75f, 0.58f, 0.26f, 0.85f), 1.5f);
    drawText(fpsText, 24.0f, 21.0f, 2.0f, Vec4(0.45f, 1.0f, 0.55f, 1.0f));

    if (state == STATE_MAIN_MENU) {
        renderMainMenu(mouseX, mouseY, hud);
    } else if (state == STATE_PLAYING) {
        renderHUD(hud);
    } else if (state == STATE_PAUSED) {
        renderHUD(hud);
        renderPauseMenu(mouseX, mouseY);
    } else if (state == STATE_RECIPES) {
        renderRecipesOverlay(mouseX, mouseY, hud);
    }

    end2D();
}

void UIRenderer::renderMainMenu(int mouseX, int mouseY, const HUDState& hud) {
    (void)hud;
    float cx = curW * 0.5f;
    float cy = curH * 0.5f;
    float pw = 540.0f;
    float ph = 390.0f;
    float px = cx - pw * 0.5f;
    float py = cy - ph * 0.5f;

    // Subtle dark vignette backdrop behind menu card
    drawRect(0.0f, 0.0f, (float)curW, (float)curH, Vec4(0.02f, 0.02f, 0.04f, 0.42f));
    drawPanel(px, py, pw, ph, Vec4(0.07f, 0.06f, 0.10f, 0.88f), Vec4(0.86f, 0.66f, 0.28f, 0.95f), 3.0f);

    drawTextCentered("ALCHEMIST'S POTION LAB", cx, py + 38.0f, 3.0f, Vec4(1.0f, 0.86f, 0.42f, 1.0f));
    drawRect(px + 40.0f, py + 84.0f, pw - 80.0f, 2.0f, Vec4(0.65f, 0.50f, 0.22f, 0.70f));

    UIButton bPlay    = { cx - 160.0f, py + 114.0f, 320.0f, 48.0f, "[1] ENTER LABORATORY", 1 };
    UIButton bRecipes = { cx - 160.0f, py + 180.0f, 320.0f, 48.0f, "[2] RECIPES & CONTROLS", 2 };
    UIButton bQuit    = { cx - 160.0f, py + 246.0f, 320.0f, 48.0f, "[3] QUIT", 3 };
    activeButtons.push_back(bPlay);
    activeButtons.push_back(bRecipes);
    activeButtons.push_back(bQuit);

    for (const auto& b : activeButtons) drawButton(b, mouseX, mouseY);

    drawTextCentered("MIX 3 INGREDIENTS TO DISCOVER 3 MAGICAL EFFECTS!", cx, py + 322.0f, 1.5f, Vec4(0.85f, 0.82f, 0.70f, 1.0f));
    drawTextCentered("CLICK A BUTTON OR PRESS [ENTER] TO START", cx, py + 350.0f, 1.5f, Vec4(0.60f, 0.72f, 0.82f, 1.0f));
}

void UIRenderer::renderHUD(const HUDState& hud) {
    if (!hud.potions) return;

    // Top-left status bar next to FPS badge (sized cleanly so zero overlap at 1280px)
    std::string camLabel = (hud.camMode == CAMERA_FPS) ? "FPS WALK" : "ORBIT";
    std::string info = "CAM:" + camLabel
                     + "  /  RECIPES:" + std::to_string(hud.potions->getDiscoveredCount()) + "/3"
                     + "  /  CANDLES:" + (hud.candlesLit ? "ON" : "OFF")
                     + "  /  FX:" + std::to_string(hud.particleCount);
    drawPanel(142.0f, 14.0f, 495.0f, 30.0f, Vec4(0.06f, 0.06f, 0.09f, 0.80f), Vec4(0.55f, 0.44f, 0.24f, 0.85f), 1.5f);
    drawText(info, 154.0f, 23.0f, 1.45f, Vec4(0.92f, 0.88f, 0.76f, 1.0f));

    // Top-right compact quick-controls strip
    std::string quick = "WASD:MOVE  RMB:LOOK  1/2/3:ADD  E:BREW  R:CLEAR  V:CAM  H:HELP";
    float qw = 490.0f;
    drawPanel((float)curW - qw - 14.0f, 14.0f, qw, 30.0f, Vec4(0.06f, 0.06f, 0.09f, 0.80f), Vec4(0.45f, 0.50f, 0.65f, 0.80f), 1.5f);
    drawText(quick, (float)curW - qw - 2.0f, 23.0f, 1.25f, Vec4(0.82f, 0.88f, 0.98f, 1.0f));

    // Subtle center reticle in FPS mode
    if (hud.camMode == CAMERA_FPS) {
        float cx = curW * 0.5f;
        float cy = curH * 0.5f;
        drawRect(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f, Vec4(1.0f, 0.90f, 0.55f, 0.65f));
    }

    // Dynamic status / effect notification banner
    if (hud.potions->bannerTimer > 0.0f && !hud.potions->statusBanner.empty()) {
        float bw = std::min((float)curW - 40.0f, std::max(460.0f, (float)hud.potions->statusBanner.size() * 9.8f + 44.0f));
        float bx = (curW - bw) * 0.5f;
        float by = (float)curH - 144.0f;
        drawPanel(bx, by, bw, 34.0f, Vec4(0.08f, 0.06f, 0.12f, 0.88f), Vec4(0.95f, 0.75f, 0.30f, 0.95f), 2.0f);
        drawTextCentered(hud.potions->statusBanner, curW * 0.5f, by + 10.0f, 1.55f, Vec4(1.0f, 0.94f, 0.72f, 1.0f));
    }

    // Bottom-center Alchemy Mixing Workbench Bar (generous 860px width for all recipe titles)
    float barW = 860.0f;
    float barH = 82.0f;
    float bx = (curW - barW) * 0.5f;
    float by = (float)curH - barH - 14.0f;
    drawPanel(bx, by, barW, barH, Vec4(0.06f, 0.06f, 0.10f, 0.86f), Vec4(0.75f, 0.58f, 0.25f, 0.92f), 2.0f);

    // Ingredient Slot 1
    Vec3 c1 = PotionSystem::getIngredientColor(hud.potions->slot1);
    Vec4 sw1 = (hud.potions->slot1 == ING_NONE) ? Vec4(0.22f, 0.22f, 0.26f, 0.9f) : Vec4(c1, 1.0f);
    drawPanel(bx + 18.0f, by + 16.0f, 22.0f, 22.0f, sw1, Vec4(0.85f, 0.75f, 0.45f, 0.9f), 1.5f);
    drawText("1:" + PotionSystem::getIngredientName(hud.potions->slot1), bx + 48.0f, by + 20.0f, 1.6f, Vec4(0.94f, 0.92f, 0.88f, 1.0f));

    drawText("+", bx + 228.0f, by + 19.0f, 2.0f, Vec4(1.0f, 0.82f, 0.35f, 1.0f));

    // Ingredient Slot 2
    Vec3 c2 = PotionSystem::getIngredientColor(hud.potions->slot2);
    Vec4 sw2 = (hud.potions->slot2 == ING_NONE) ? Vec4(0.22f, 0.22f, 0.26f, 0.9f) : Vec4(c2, 1.0f);
    drawPanel(bx + 256.0f, by + 16.0f, 22.0f, 22.0f, sw2, Vec4(0.85f, 0.75f, 0.45f, 0.9f), 1.5f);
    drawText("2:" + PotionSystem::getIngredientName(hud.potions->slot2), bx + 286.0f, by + 20.0f, 1.6f, Vec4(0.94f, 0.92f, 0.88f, 1.0f));

    drawText("->", bx + 462.0f, by + 20.0f, 1.8f, Vec4(1.0f, 0.82f, 0.35f, 1.0f));

    // Result / Brew status
    Vec4 resCol = hud.potions->canBrew() ? Vec4(1.0f, 0.88f, 0.32f, 1.0f) : Vec4(0.76f, 0.86f, 0.96f, 1.0f);
    drawText(hud.potions->currentPotionTitle, bx + 494.0f, by + 20.0f, 1.55f, resCol);

    // Bottom ingredient key legend strip
    drawTextCentered("[1] CRIMSON EMBER (RED)    [2] FROST DEW (BLUE)    [3] VENOM LEAF (GREEN)    [E] BREW",
                     curW * 0.5f, by + 54.0f, 1.45f, Vec4(0.78f, 0.80f, 0.84f, 1.0f));
}

void UIRenderer::renderPauseMenu(int mouseX, int mouseY) {
    float cx = curW * 0.5f;
    float cy = curH * 0.5f;
    float pw = 400.0f;
    float ph = 330.0f;
    float px = cx - pw * 0.5f;
    float py = cy - ph * 0.5f;

    drawRect(0.0f, 0.0f, (float)curW, (float)curH, Vec4(0.02f, 0.02f, 0.04f, 0.55f));
    drawPanel(px, py, pw, ph, Vec4(0.08f, 0.07f, 0.11f, 0.93f), Vec4(0.86f, 0.66f, 0.28f, 1.0f), 2.5f);
    drawTextCentered("GAME PAUSED", cx, py + 24.0f, 2.6f, Vec4(1.0f, 0.86f, 0.42f, 1.0f));

    UIButton bResume  = { cx - 140.0f, py + 76.0f,  280.0f, 44.0f, "RESUME GAME", 10 };
    UIButton bRecipes = { cx - 140.0f, py + 132.0f, 280.0f, 44.0f, "RECIPES & CONTROLS", 11 };
    UIButton bMenu    = { cx - 140.0f, py + 188.0f, 280.0f, 44.0f, "MAIN MENU", 12 };
    UIButton bQuit    = { cx - 140.0f, py + 244.0f, 280.0f, 44.0f, "QUIT", 13 };
    activeButtons.push_back(bResume);
    activeButtons.push_back(bRecipes);
    activeButtons.push_back(bMenu);
    activeButtons.push_back(bQuit);

    for (const auto& b : activeButtons) drawButton(b, mouseX, mouseY);
}

void UIRenderer::renderRecipesOverlay(int mouseX, int mouseY, const HUDState& hud) {
    float cx = curW * 0.5f;
    float cy = curH * 0.5f;
    float pw = 760.0f;
    float ph = 490.0f;
    float px = cx - pw * 0.5f;
    float py = cy - ph * 0.5f;

    drawRect(0.0f, 0.0f, (float)curW, (float)curH, Vec4(0.02f, 0.02f, 0.04f, 0.65f));
    drawPanel(px, py, pw, ph, Vec4(0.07f, 0.06f, 0.10f, 0.94f), Vec4(0.88f, 0.68f, 0.30f, 1.0f), 3.0f);

    drawTextCentered("ALCHEMIST'S GRIMOIRE - RECIPES & CONTROLS", cx, py + 22.0f, 2.3f, Vec4(1.0f, 0.86f, 0.42f, 1.0f));

    // Section 1: 3 Magical Potion Combinations
    float ry = py + 66.0f;
    drawText("POTION RECIPES (ADD 2 INGREDIENTS, THEN PRESS [E]):", px + 28.0f, ry, 1.65f, Vec4(0.95f, 0.78f, 0.38f, 1.0f));

    bool d0 = hud.potions ? hud.potions->discovered[0] : false;
    bool d1 = hud.potions ? hud.potions->discovered[1] : false;
    bool d2 = hud.potions ? hud.potions->discovered[2] : false;

    drawText(std::string(d0 ? "[OK] " : "[  ] ") + "EMBER [1] + VENOM [3] = INFERNO",
             px + 34.0f, ry + 30.0f, 1.75f, Vec4(1.0f, 0.55f, 0.28f, 1.0f));
    drawText(std::string(d1 ? "[OK] " : "[  ] ") + "EMBER [1] + FROST [2] = STARLIGHT",
             px + 34.0f, ry + 58.0f, 1.75f, Vec4(0.88f, 0.55f, 1.0f, 1.0f));
    drawText(std::string(d2 ? "[OK] " : "[  ] ") + "FROST [2] + VENOM [3] = GLACIAL MIST",
             px + 34.0f, ry + 86.0f, 1.75f, Vec4(0.42f, 0.94f, 1.0f, 1.0f));

    drawRect(px + 28.0f, ry + 114.0f, pw - 56.0f, 2.0f, Vec4(0.55f, 0.45f, 0.25f, 0.70f));

    // Section 2: Controls Guide
    float cy2 = ry + 130.0f;
    drawText("CONTROLS:", px + 28.0f, cy2, 1.65f, Vec4(0.95f, 0.78f, 0.38f, 1.0f));
    const char* lines[] = {
        "W / A / S / D             : WALK AROUND THE ALCHEMY LABORATORY",
        "RIGHT MOUSE (DRAG) / [F]  : LOOK AROUND / TOGGLE FREE MOUSE LOOK",
        "[V] / MOUSE SCROLL        : SWITCH FPS WALK <-> ORBIT CAMERA / ZOOM",
        "[1] [2] [3] OR LEFT-CLICK : POUR INGREDIENT INTO MAIN BREWING FLASK",
        "[E] OR [ENTER]            : BREW CURRENT MIXTURE (TRIGGER EFFECT!)",
        "LEFT-CLICK DRAG ON FLASK  : GRAB & SHAKE MAIN FLASK (SLOSH PHYSICS)",
        "[SPACE] / [+] [-]         : STIR POTION / ADJUST LIQUID FILL LEVEL",
        "[L] / [R] / [H] / [ESC]   : CANDLES ON-OFF / CLEAR FLASK / HELP / PAUSE"
    };
    for (int i = 0; i < 8; ++i) {
        drawText(lines[i], px + 34.0f, cy2 + 28.0f + i * 24.0f, 1.48f, Vec4(0.88f, 0.90f, 0.94f, 1.0f));
    }

    UIButton bClose = { cx - 120.0f, py + ph - 62.0f, 240.0f, 42.0f, "BACK [H / ESC]", 20 };
    activeButtons.push_back(bClose);
    drawButton(bClose, mouseX, mouseY);
}
