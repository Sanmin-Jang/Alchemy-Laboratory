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

Texture::Texture(Texture&& other) noexcept
    : id(other.id), width(other.width), height(other.height) {
    other.id = 0;
    other.width = 0;
    other.height = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (id != 0) {
            glDeleteTextures(1, &id);
        }
        id = other.id;
        width = other.width;
        height = other.height;

        other.id = 0;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

void Texture::bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::initFromRGBA(int w, int h, const std::vector<unsigned char>& data, bool nearestFilter) {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
    width = w;
    height = h;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    if (nearestFilter) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture Texture::createFromRGBA(int w, int h, const std::vector<unsigned char>& data, bool nearestFilter) {
    Texture tex;
    tex.initFromRGBA(w, h, data, nearestFilter);
    return tex;
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
            rgba[dstIndex + 0] = rawData[srcIndex + 2];
            rgba[dstIndex + 1] = rawData[srcIndex + 1];
            rgba[dstIndex + 2] = rawData[srcIndex + 0];
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

            float dist = std::sqrt(std::pow(u - 0.5f, 2.0f) + std::pow(v * 0.2f - 0.1f, 2.0f));
            float ring = std::sin(dist * 60.0f + std::sin(u * 20.0f) * 1.5f);
            float fiber = std::sin(y * 1.8f) * 0.1f + std::sin(x * 0.05f) * 0.05f;
            float val = std::max(0.0f, std::min(1.0f, 0.5f + 0.35f * ring + fiber));

            int idx = (y * size + x) * 4;
            data[idx + 0] = (unsigned char)(val * 78.0f + 42.0f);
            data[idx + 1] = (unsigned char)(val * 46.0f + 24.0f);
            data[idx + 2] = (unsigned char)(val * 26.0f + 14.0f);
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
            float localU = std::fmod((u + rowOffset) * brickCols, 1.0f);
            float localV = std::fmod(v * brickRows, 1.0f);

            bool isMortar = (localU < 0.05f || localV < 0.07f);
            float noise = (std::sin(x * 0.3f) + std::cos(y * 0.4f)) * 0.10f
                        + std::sin((x + y) * 0.17f) * 0.05f;
            float base = isMortar ? 0.24f : (0.56f + noise);
            base = std::max(0.15f, std::min(1.0f, base));

            int idx = (y * size + x) * 4;
            data[idx + 0] = (unsigned char)(base * 92.0f);
            data[idx + 1] = (unsigned char)(base * 88.0f);
            data[idx + 2] = (unsigned char)(base * 100.0f);
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

            float stain = std::sin(u * 8.0f) * std::cos(v * 9.0f) * 0.14f +
                          std::sin(u * 25.0f + v * 30.0f) * 0.04f;
            float edgeVig = 1.0f - 1.1f * (std::pow(u - 0.5f, 2.0f) + std::pow(v - 0.5f, 2.0f));
            float factor = std::max(0.42f, std::min(1.0f, edgeVig + stain));

            float ink = 0.0f;
            bool borderV = (std::fabs(u - 0.08f) < 0.008f || std::fabs(u - 0.92f) < 0.008f) && (v > 0.08f && v < 0.92f);
            bool borderH = (std::fabs(v - 0.08f) < 0.008f || std::fabs(v - 0.92f) < 0.008f) && (u > 0.08f && u < 0.92f);
            if (borderV || borderH) ink = 0.65f;

            float dc = std::sqrt(std::pow(u - 0.30f, 2.0f) + std::pow(v - 0.65f, 2.0f));
            if (std::fabs(dc - 0.14f) < 0.009f || std::fabs(dc - 0.09f) < 0.006f) ink = 0.72f;

            if (u > 0.50f && u < 0.86f && v > 0.22f && v < 0.80f) {
                float lineWave = std::sin(v * 75.0f);
                float wordBreak = std::sin(u * 42.0f + std::floor(v * 12.0f) * 2.1f);
                if (lineWave > 0.65f && wordBreak > -0.25f) ink = 0.55f;
            }

            int idx = (y * size + x) * 4;
            data[idx + 0] = (unsigned char)std::max(0.0f, std::min(255.0f, factor * 195.0f * (1.0f - ink * 0.72f)));
            data[idx + 1] = (unsigned char)std::max(0.0f, std::min(255.0f, factor * 168.0f * (1.0f - ink * 0.78f)));
            data[idx + 2] = (unsigned char)std::max(0.0f, std::min(255.0f, factor * 122.0f * (1.0f - ink * 0.82f)));
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
            float angle = std::atan2(dy, dx);

            float ring1 = std::exp(-std::pow((r - maxR * 0.95f) / 2.5f, 2.0f));
            float ring2 = std::exp(-std::pow((r - maxR * 0.82f) / 1.8f, 2.0f));
            float ring3 = std::exp(-std::pow((r - maxR * 0.60f) / 2.0f, 2.0f));
            float ring4 = std::exp(-std::pow((r - maxR * 0.35f) / 1.5f, 2.0f));

            float spoke = std::pow(std::cos(angle * 8.0f), 16.0f) * (r < maxR * 0.95f && r > maxR * 0.35f ? 1.0f : 0.0f);
            float notches = std::pow(std::cos(angle * 32.0f), 8.0f) * (r < maxR * 0.95f && r > maxR * 0.82f ? 0.7f : 0.0f);

            float intensity = std::min(1.0f, ring1 + ring2 + ring3 + ring4 + spoke * 0.8f + notches * 0.6f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = (unsigned char)(intensity * 255.0f);
            data[idx + 1] = (unsigned char)(intensity * 255.0f);
            data[idx + 2] = (unsigned char)(intensity * 255.0f);
            data[idx + 3] = (unsigned char)(intensity * 240.0f);
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

Texture Texture::createBookLeatherTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = (float)x / size;
            float v = (float)y / size;
            float grain = (std::sin(x * 0.9f + std::cos(y * 0.7f)) + std::sin(y * 1.4f)) * 0.07f;
            bool goldTrim = (u < 0.08f || u > 0.92f || v < 0.08f || v > 0.92f) &&
                            (u > 0.04f && u < 0.96f && v > 0.04f && v < 0.96f);
            float dc = std::sqrt(std::pow(u - 0.5f, 2.0f) + std::pow(v - 0.5f, 2.0f));
            bool goldEmblem = std::fabs(dc - 0.22f) < 0.022f;

            int idx = (y * size + x) * 4;
            if (goldTrim || goldEmblem) {
                data[idx + 0] = 218;
                data[idx + 1] = 178;
                data[idx + 2] = 74;
            } else {
                float shade = std::max(0.3f, std::min(1.0f, 0.75f + grain));
                data[idx + 0] = (unsigned char)(shade * 165.0f);
                data[idx + 1] = (unsigned char)(shade * 155.0f);
                data[idx + 2] = (unsigned char)(shade * 150.0f);
            }
            data[idx + 3] = 255;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createWindowTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = (float)x / size;
            float v = (float)y / size;
            float dx = (u - 0.5f) * 2.0f;
            bool inArch = (v < 0.62f) ? (std::fabs(dx) < 0.82f)
                                      : (dx * dx + std::pow((v - 0.62f) / 0.34f, 2.0f) < 0.68f);
            bool bar = (std::fabs(u - 0.5f) < 0.03f) ||
                       (std::fabs(v - 0.36f) < 0.025f) ||
                       (std::fabs(v - 0.62f) < 0.025f);

            int idx = (y * size + x) * 4;
            if (!inArch || bar) {
                data[idx + 0] = 18;
                data[idx + 1] = 20;
                data[idx + 2] = 28;
                data[idx + 3] = 255;
            } else {
                float moonDist = std::sqrt(std::pow(u - 0.60f, 2.0f) + std::pow(v - 0.72f, 2.0f));
                float moon = std::exp(-moonDist * moonDist * 55.0f);
                float sky = 0.25f + 0.45f * v + moon * 0.95f;
                data[idx + 0] = (unsigned char)std::min(255.0f, sky * 115.0f + moon * 130.0f);
                data[idx + 1] = (unsigned char)std::min(255.0f, sky * 155.0f + moon * 110.0f);
                data[idx + 2] = (unsigned char)std::min(255.0f, sky * 235.0f + moon * 50.0f);
                data[idx + 3] = 255;
            }
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}

Texture Texture::createParticleTexture(int size) {
    Texture tex;
    std::vector<unsigned char> data(size * size * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = ((float)x + 0.5f) / size * 2.0f - 1.0f;
            float v = ((float)y + 0.5f) / size * 2.0f - 1.0f;
            float r2 = u * u + v * v;
            float alpha = (r2 >= 1.0f) ? 0.0f : std::exp(-r2 * 3.8f) * (1.0f - r2);
            unsigned char c = (unsigned char)std::min(255.0f, (0.80f + 0.20f * alpha) * 255.0f);
            unsigned char a = (unsigned char)std::min(255.0f, alpha * 255.0f);

            int idx = (y * size + x) * 4;
            data[idx + 0] = c;
            data[idx + 1] = c;
            data[idx + 2] = c;
            data[idx + 3] = a;
        }
    }

    tex.initFromRGBA(size, size, data);
    return tex;
}
