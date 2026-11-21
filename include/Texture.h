#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>
#include <vector>

class Texture {
public:
    GLuint id;
    int width;
    int height;

    Texture();
    ~Texture();

    // Prevent double-free from shallow copy
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Move semantics for proper OpenGL texture handle transfer
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(GLuint slot = 0) const;
    void unbind() const;

    // Load from standard 24-bit or 32-bit uncompressed BMP file
    bool loadBMP(const std::string& path);

    // Procedural texture generators with mipmaps
    static Texture createWoodTexture(int size = 512);
    static Texture createStoneWallTexture(int size = 512);
    static Texture createParchmentTexture(int size = 512);
    static Texture createRuneCircleTexture(int size = 512);
    static Texture createMortarStoneTexture(int size = 256);
    static Texture createBookLeatherTexture(int size = 256);
    static Texture createWindowTexture(int size = 256);
    static Texture createParticleTexture(int size = 128);
    static Texture createFromRGBA(int w, int h, const std::vector<unsigned char>& data, bool nearestFilter = false);

private:
    void initFromRGBA(int w, int h, const std::vector<unsigned char>& data, bool nearestFilter = false);
};

#endif // TEXTURE_H