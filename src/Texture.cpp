#include "Texture.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

Texture::Texture() : id(0), width(0), height(0) {}

Texture::~Texture() {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

void Texture::bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::initFromRGBA(int w, int h, const std::vector<unsigned char>& data) {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
    width = w;
    height = h;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::loadBMP(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Texture] Failed to open BMP: " << path << std::endl;
        return false;
    }

    unsigned char header[54];
    file.read(reinterpret_cast<char*>(header), 54);
    if (file.gcount() < 54 || header[0] != 'B' || header[1] != 'M') {
        std::cerr << "[Texture] Invalid BMP format: " << path << std::endl;
        return false;
    }

    int dataPos   = *(int*)&(header[0x0A]);
    int imageSize = *(int*)&(header[0x22]);
    width         = *(int*)&(header[0x12]);
    height        = *(int*)&(header[0x16]);
    short bpp     = *(short*)&(header[0x1C]);

    if (imageSize == 0) imageSize = width * height * (bpp / 8);
    if (dataPos == 0)   dataPos = 54;

    file.seekg(dataPos);
    std::vector<unsigned char> rawData(imageSize);
    file.read(reinterpret_cast<char*>(rawData.data()), imageSize);

    std::vector<unsigned char> rgba(width * height * 4);
    int bytesPerPixel = bpp / 8;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcIndex = (y * width + x) * bytesPerPixel;
            int dstIndex = (y * width + x) * 4;
            rgba[dstIndex + 0] = rawData[srcIndex + 2]; // R
            rgba[dstIndex + 1] = rawData[srcIndex + 1]; // G
            rgba[dstIndex + 2] = rawData[srcIndex + 0]; // B
            rgba[dstIndex + 3] = (bytesPerPixel == 4) ? rawData[srcIndex + 3] : 255;
        }
    }

    initFromRGBA(width, height, rgba);
    return true;
}

Texture Texture::createWoodTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = (float)x / size;
            float v = (float)y / size;

            // Wood grain rings + fiber noise
            float dist = std::sqrt(std::pow(u - 0.5f, 2.0f) + std::pow(v * 0.2f - 0.1f, 2.0f));
            float ring = std::sin(dist * 60.0f + std::sin(u * 20.0f) * 1.5f);
            float fiber = std::sin(y * 1.8f) * 0.1f + std::sin(x * 0.05f) * 0.05f;
            float val = 0.5f + 0.35f * ring + fiber;
            val = std::max(0.0f, std::min(1.0f, val));

            // Dark antique oak palette
            unsigned char r = (unsigned char)(val * 75.0f + 40.0f);
            unsigned char g = (unsigned char)(val * 45.0f + 25.0f);
            unsigned char b = (unsigned char)(val * 25.0f + 15.0f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = r;
            data[idx + 1] = g;
            data[idx + 2] = b;
            data[idx + 3] = 255;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createStoneWallTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    int brickRows = 8;
    int brickCols = 4;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = (float)x / size;
            float v = (float)y / size;

            int row = (int)(v * brickRows);
            float rowOffset = (row % 2 == 1) ? 0.5f / brickCols : 0.0f;
            int col = (int)((u + rowOffset) * brickCols);

            float localU = std::fmod((u + rowOffset) * brickCols, 1.0f);
            float localV = std::fmod(v * brickRows, 1.0f);

            // Mortar joints
            bool isMortar = (localU < 0.05f || localV < 0.07f);

            float noise = (std::sin(x * 0.3f) + std::cos(y * 0.4f)) * 0.1f;
            float base = isMortar ? 0.25f : (0.55f + noise);

            unsigned char r = (unsigned char)(base * 90.0f);
            unsigned char g = (unsigned char)(base * 85.0f);
            unsigned char b = (unsigned char)(base * 95.0f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = r;
            data[idx + 1] = g;
            data[idx + 2] = b;
            data[idx + 3] = 255;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createParchmentTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = (float)x / size;
            float v = (float)y / size;

            float stain = std::sin(u * 8.0f) * std::cos(v * 9.0f) * 0.15f +
                          std::sin(u * 25.0f + v * 30.0f) * 0.05f;
            float edgeVig = 1.0f - 1.2f * (std::pow(u - 0.5f, 2.0f) + std::pow(v - 0.5f, 2.0f));
            float factor = std::max(0.4f, std::min(1.0f, edgeVig + stain));

            unsigned char r = (unsigned char)(factor * 180.0f);
            unsigned char g = (unsigned char)(factor * 155.0f);
            unsigned char b = (unsigned char)(factor * 115.0f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = r;
            data[idx + 1] = g;
            data[idx + 2] = b;
            data[idx + 3] = 255;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createRuneCircleTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    float center = size * 0.5f;
    float maxR = size * 0.46f;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = (float)x - center;
            float dy = (float)y - center;
            float r = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx); // [-PI, PI]

            float intensity = 0.0f;

            // Concentric rings
            float ring1 = std::exp(-std::pow((r - maxR * 0.95f) / 2.5f, 2.0f));
            float ring2 = std::exp(-std::pow((r - maxR * 0.82f) / 1.8f, 2.0f));
            float ring3 = std::exp(-std::pow((r - maxR * 0.60f) / 2.0f, 2.0f));
            float ring4 = std::exp(-std::pow((r - maxR * 0.35f) / 1.5f, 2.0f));

            // Radial arcane spokes (8-fold symmetry)
            float spoke = std::pow(std::cos(angle * 8.0f), 16.0f) * (r < maxR * 0.95f && r > maxR * 0.35f ? 1.0f : 0.0f);

            // Outer runic notches
            float notches = std::pow(std::cos(angle * 32.0f), 8.0f) * (r < maxR * 0.95f && r > maxR * 0.82f ? 0.7f : 0.0f);

            intensity = ring1 + ring2 + ring3 + ring4 + spoke * 0.8f + notches * 0.6f;
            intensity = std::min(1.0f, intensity);

            // Magical emerald / cyan rune glow
            unsigned char cr = (unsigned char)(intensity * 50.0f);
            unsigned char cg = (unsigned char)(intensity * 255.0f);
            unsigned char cb = (unsigned char)(intensity * 180.0f);
            unsigned char ca = (unsigned char)(intensity * 230.0f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = cr;
            data[idx + 1] = cg;
            data[idx + 2] = cb;
            data[idx + 3] = ca;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createMortarStoneTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float noise = (std::sin(x * 0.8f) * std::cos(y * 0.7f) +
                           std::sin(x * 2.3f + y * 1.7f) * 0.5f) * 0.15f;
            float val = std::max(0.0f, std::min(1.0f, 0.45f + noise));

            unsigned char c = (unsigned char)(val * 200.0f);
            int idx = (y * size + x) * 4;
            data[idx + 0] = c;
            data[idx + 1] = (unsigned char)(c * 0.95f);
            data[idx + 2] = (unsigned char)(c * 0.90f);
            data[idx + 3] = 255;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}
