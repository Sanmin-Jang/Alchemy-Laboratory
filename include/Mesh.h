#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <vector>
#include "math3d.h"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoords;

    Vertex() : position(0, 0, 0), normal(0, 1, 0), texCoords(0, 0) {}
    Vertex(const Vec3& p, const Vec3& n, const Vec2& uv)
        : position(p), normal(n), texCoords(uv) {}
};

class Mesh {
public:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLsizei indexCount;

    Mesh();
    ~Mesh();

    // Prevent double-free from shallow copy
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Move semantics for proper OpenGL resource transfer
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void draw() const;

    // Procedural primitive mesh builders
    static Mesh createBox(float width, float height, float depth, float uScale = 1.0f, float vScale = 1.0f);
    static Mesh createCylinder(float bottomRadius, float topRadius, float height, int slices = 32, bool capBottom = true, bool capTop = true);
    static Mesh createSphere(float radius, int rings = 24, int sectors = 32);
    static Mesh createPlane(float width, float depth, float uTiling = 1.0f, float vTiling = 1.0f);
    static Mesh createErlenmeyerFlask(float baseRadius, float shoulderRadius, float neckRadius, float totalHeight, float neckHeight, int slices = 48);
    static Mesh createLiquidVolume(float baseRadius, float shoulderRadius, float neckRadius, float totalHeight, float neckHeight, int slices = 48);
    static Mesh createTeardropFlame(float radius = 0.05f, float height = 0.18f, int slices = 16);
    static Mesh createDisk(float radius = 0.25f, int rings = 16, int sectors = 48);
    static Mesh createBillboard(float size = 1.0f);
    static Mesh createTorus(float majorRadius, float minorRadius, int majorSegs = 32, int minorSegs = 16);
};

#endif // MESH_H
