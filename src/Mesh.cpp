#include "Mesh.h"
#include <cmath>

Mesh::Mesh() : vao(0), vbo(0), ebo(0), indexCount(0) {}

Mesh::~Mesh() {
    if (ebo != 0) glDeleteBuffers(1, &ebo);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (ebo != 0) glDeleteBuffers(1, &ebo);
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);

        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        indexCount = other.indexCount;

        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (ebo != 0) glDeleteBuffers(1, &ebo);

    indexCount = (GLsizei)indices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Normal (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // TexCoords (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (vao != 0 && indexCount > 0) {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

Mesh Mesh::createBox(float width, float height, float depth, float uScale, float vScale) {
    Mesh mesh;
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    float d2 = depth * 0.5f;

    std::vector<Vertex> vertices = {
        // Front
        { {-w2, -h2,  d2}, { 0,  0,  1}, {0.0f * uScale, 0.0f * vScale} },
        { { w2, -h2,  d2}, { 0,  0,  1}, {1.0f * uScale, 0.0f * vScale} },
        { { w2,  h2,  d2}, { 0,  0,  1}, {1.0f * uScale, 1.0f * vScale} },
        { {-w2,  h2,  d2}, { 0,  0,  1}, {0.0f * uScale, 1.0f * vScale} },
        // Back
        { { w2, -h2, -d2}, { 0,  0, -1}, {0.0f * uScale, 0.0f * vScale} },
        { {-w2, -h2, -d2}, { 0,  0, -1}, {1.0f * uScale, 0.0f * vScale} },
        { {-w2,  h2, -d2}, { 0,  0, -1}, {1.0f * uScale, 1.0f * vScale} },
        { { w2,  h2, -d2}, { 0,  0, -1}, {0.0f * uScale, 1.0f * vScale} },
        // Top
        { {-w2,  h2,  d2}, { 0,  1,  0}, {0.0f * uScale, 0.0f * vScale} },
        { { w2,  h2,  d2}, { 0,  1,  0}, {1.0f * uScale, 0.0f * vScale} },
        { { w2,  h2, -d2}, { 0,  1,  0}, {1.0f * uScale, 1.0f * vScale} },
        { {-w2,  h2, -d2}, { 0,  1,  0}, {0.0f * uScale, 1.0f * vScale} },
        // Bottom
        { {-w2, -h2, -d2}, { 0, -1,  0}, {0.0f * uScale, 0.0f * vScale} },
        { { w2, -h2, -d2}, { 0, -1,  0}, {1.0f * uScale, 0.0f * vScale} },
        { { w2, -h2,  d2}, { 0, -1,  0}, {1.0f * uScale, 1.0f * vScale} },
        { {-w2, -h2,  d2}, { 0, -1,  0}, {0.0f * uScale, 1.0f * vScale} },
        // Right
        { { w2, -h2,  d2}, { 1,  0,  0}, {0.0f * uScale, 0.0f * vScale} },
        { { w2, -h2, -d2}, { 1,  0,  0}, {1.0f * uScale, 0.0f * vScale} },
        { { w2,  h2, -d2}, { 1,  0,  0}, {1.0f * uScale, 1.0f * vScale} },
        { { w2,  h2,  d2}, { 1,  0,  0}, {0.0f * uScale, 1.0f * vScale} },
        // Left
        { {-w2, -h2, -d2}, {-1,  0,  0}, {0.0f * uScale, 0.0f * vScale} },
        { {-w2, -h2,  d2}, {-1,  0,  0}, {1.0f * uScale, 0.0f * vScale} },
        { {-w2,  h2,  d2}, {-1,  0,  0}, {1.0f * uScale, 1.0f * vScale} },
        { {-w2,  h2, -d2}, {-1,  0,  0}, {0.0f * uScale, 1.0f * vScale} }
    };

    std::vector<unsigned int> indices;
    for (int i = 0; i < 6; ++i) {
        int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createPlane(float width, float depth, float uTiling, float vTiling) {
    Mesh mesh;
    float w2 = width * 0.5f;
    float d2 = depth * 0.5f;

    std::vector<Vertex> vertices = {
        { {-w2, 0.0f,  d2}, {0, 1, 0}, {0.0f,           0.0f} },
        { { w2, 0.0f,  d2}, {0, 1, 0}, {uTiling,        0.0f} },
        { { w2, 0.0f, -d2}, {0, 1, 0}, {uTiling,        vTiling} },
        { {-w2, 0.0f, -d2}, {0, 1, 0}, {0.0f,           vTiling} }
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };
    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createCylinder(float bottomRadius, float topRadius, float height, int slices, bool capBottom, bool capTop) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Body vertices
    for (int i = 0; i <= slices; ++i) {
        float theta = (float)i / slices * 2.0f * (float)M_PI;
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);
        float u = (float)i / slices;

        // Slanted normal for conical cylinders
        float dr = bottomRadius - topRadius;
        Vec3 normal(cosT, dr / height, sinT);
        normal = normal.normalized();

        // Bottom ring
        vertices.emplace_back(Vec3(bottomRadius * cosT, 0.0f, bottomRadius * sinT), normal, Vec2(u, 0.0f));
        // Top ring
        vertices.emplace_back(Vec3(topRadius * cosT, height, topRadius * sinT), normal, Vec2(u, 1.0f));
    }

    for (int i = 0; i < slices; ++i) {
        int b1 = i * 2;
        int t1 = b1 + 1;
        int b2 = (i + 1) * 2;
        int t2 = b2 + 1;

        indices.push_back(b1);
        indices.push_back(b2);
        indices.push_back(t2);

        indices.push_back(b1);
        indices.push_back(t2);
        indices.push_back(t1);
    }

    // Caps
    if (capBottom) {
        int centerIdx = (int)vertices.size();
        vertices.emplace_back(Vec3(0, 0, 0), Vec3(0, -1, 0), Vec2(0.5f, 0.5f));
        for (int i = 0; i <= slices; ++i) {
            float theta = (float)i / slices * 2.0f * (float)M_PI;
            vertices.emplace_back(Vec3(bottomRadius * std::cos(theta), 0.0f, bottomRadius * std::sin(theta)),
                                  Vec3(0, -1, 0), Vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
        }
        for (int i = 0; i < slices; ++i) {
            indices.push_back(centerIdx);
            indices.push_back(centerIdx + 1 + (i + 1));
            indices.push_back(centerIdx + 1 + i);
        }
    }

    if (capTop) {
        int centerIdx = (int)vertices.size();
        vertices.emplace_back(Vec3(0, height, 0), Vec3(0, 1, 0), Vec2(0.5f, 0.5f));
        for (int i = 0; i <= slices; ++i) {
            float theta = (float)i / slices * 2.0f * (float)M_PI;
            vertices.emplace_back(Vec3(topRadius * std::cos(theta), height, topRadius * std::sin(theta)),
                                  Vec3(0, 1, 0), Vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
        }
        for (int i = 0; i < slices; ++i) {
            indices.push_back(centerIdx);
            indices.push_back(centerIdx + 1 + i);
            indices.push_back(centerIdx + 1 + (i + 1));
        }
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createSphere(float radius, int rings, int sectors) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int r = 0; r <= rings; ++r) {
        float phi = (float)r / rings * (float)M_PI;
        for (int s = 0; s <= sectors; ++s) {
            float theta = (float)s / sectors * 2.0f * (float)M_PI;

            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);

            Vec3 pos(x, y, z);
            Vec3 normal = pos.normalized();
            Vec2 uv((float)s / sectors, (float)r / rings);

            vertices.emplace_back(pos, normal, uv);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            int cur = r * (sectors + 1) + s;
            int next = cur + sectors + 1;

            indices.push_back(cur);
            indices.push_back(next);
            indices.push_back(next + 1);

            indices.push_back(cur);
            indices.push_back(next + 1);
            indices.push_back(cur + 1);
        }
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createErlenmeyerFlask(float baseRadius, float shoulderRadius, float neckRadius, float totalHeight, float neckHeight, int slices) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Profile points along Y: (radius, y)
    float bodyHeight = totalHeight - neckHeight;
    std::vector<std::pair<float, float>> profile = {
        { baseRadius, 0.0f },
        { baseRadius * 0.95f, 0.02f },
        { shoulderRadius, bodyHeight },
        { neckRadius, bodyHeight + 0.03f },
        { neckRadius, totalHeight },
        { neckRadius * 1.15f, totalHeight + 0.02f }
    };

    int rings = (int)profile.size();
    for (int r = 0; r < rings; ++r) {
        float rad = profile[r].first;
        float y = profile[r].second;
        float v = y / totalHeight;

        for (int s = 0; s <= slices; ++s) {
            float theta = (float)s / slices * 2.0f * (float)M_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);
            float u = (float)s / slices;

            Vec3 pos(rad * cosT, y, rad * sinT);
            Vec3 norm(cosT, 0.15f, sinT);
            norm = norm.normalized();

            vertices.emplace_back(pos, norm, Vec2(u, v));
        }
    }

    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < slices; ++s) {
            int cur = r * (slices + 1) + s;
            int next = cur + slices + 1;

            indices.push_back(cur);
            indices.push_back(next);
            indices.push_back(next + 1);

            indices.push_back(cur);
            indices.push_back(next + 1);
            indices.push_back(cur + 1);
        }
    }

    // Bottom cap
    int centerBottom = (int)vertices.size();
    vertices.emplace_back(Vec3(0, 0, 0), Vec3(0, -1, 0), Vec2(0.5f, 0.5f));
    for (int s = 0; s <= slices; ++s) {
        float theta = (float)s / slices * 2.0f * (float)M_PI;
        vertices.emplace_back(Vec3(baseRadius * std::cos(theta), 0.0f, baseRadius * std::sin(theta)),
                              Vec3(0, -1, 0), Vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
    }
    for (int s = 0; s < slices; ++s) {
        indices.push_back(centerBottom);
        indices.push_back(centerBottom + 1 + (s + 1));
        indices.push_back(centerBottom + 1 + s);
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createLiquidVolume(float baseRadius, float shoulderRadius, float neckRadius, float totalHeight, float neckHeight, int slices) {
    // Inner liquid geometry scaled slightly inside the glass
    float margin = 0.96f;
    return createErlenmeyerFlask(baseRadius * margin, shoulderRadius * margin, neckRadius * margin, totalHeight * margin, neckHeight * margin, slices);
}

Mesh Mesh::createTeardropFlame(float radius, float height, int slices) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    int rings = 12;
    for (int r = 0; r <= rings; ++r) {
        float t = (float)r / rings;
        float y = t * height;
        float w = std::sin(t * (float)M_PI) * (1.0f - 0.45f * t) * radius;

        for (int s = 0; s <= slices; ++s) {
            float theta = (float)s / slices * 2.0f * (float)M_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            Vec3 pos(w * cosT, y, w * sinT);
            Vec3 norm(cosT, 0.2f, sinT);
            norm = norm.normalized();
            Vec2 uv((float)s / slices, t);

            vertices.emplace_back(pos, norm, uv);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < slices; ++s) {
            int cur = r * (slices + 1) + s;
            int next = cur + slices + 1;

            indices.push_back(cur);
            indices.push_back(next);
            indices.push_back(next + 1);

            indices.push_back(cur);
            indices.push_back(next + 1);
            indices.push_back(cur + 1);
        }
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createDisk(float radius, int rings, int sectors) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Center vertex
    vertices.emplace_back(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.5f, 0.5f));

    for (int r = 1; r <= rings; ++r) {
        float frac = (float)r / rings;
        float curRad = radius * frac;
        for (int s = 0; s <= sectors; ++s) {
            float theta = (float)s / sectors * 2.0f * (float)M_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            Vec3 pos(curRad * cosT, 0.0f, curRad * sinT);
            Vec3 norm(0.0f, 1.0f, 0.0f);
            Vec2 uv(0.5f + 0.5f * frac * cosT, 0.5f + 0.5f * frac * sinT);

            vertices.emplace_back(pos, norm, uv);
        }
    }

    // Inner ring connected to center
    for (int s = 0; s < sectors; ++s) {
        indices.push_back(0);
        indices.push_back(1 + s);
        indices.push_back(1 + s + 1);
    }

    // Concentric ring strips
    for (int r = 1; r < rings; ++r) {
        int r1 = 1 + (r - 1) * (sectors + 1);
        int r2 = 1 + r * (sectors + 1);
        for (int s = 0; s < sectors; ++s) {
            int a = r1 + s;
            int b = r1 + s + 1;
            int c = r2 + s + 1;
            int d = r2 + s;

            indices.push_back(a);
            indices.push_back(d);
            indices.push_back(c);

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
        }
    }

    mesh.setupMesh(vertices, indices);
    return mesh;
}

Mesh Mesh::createBillboard(float size) {
    Mesh mesh;
    float h = size * 0.5f;
    std::vector<Vertex> vertices = {
        { {-h, -h, 0.0f}, {0, 0, 1}, {0.0f, 0.0f} },
        { { h, -h, 0.0f}, {0, 0, 1}, {1.0f, 0.0f} },
        { { h,  h, 0.0f}, {0, 0, 1}, {1.0f, 1.0f} },
        { {-h,  h, 0.0f}, {0, 0, 1}, {0.0f, 1.0f} }
    };
    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };
    mesh.setupMesh(vertices, indices);
    return mesh;
}
