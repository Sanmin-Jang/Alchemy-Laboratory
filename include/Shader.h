#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include "math3d.h"

class Shader {
public:
    GLuint id;

    Shader();
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const Vec2& v) const;
    void setVec3(const std::string& name, const Vec3& v) const;
    void setVec4(const std::string& name, const Vec4& v) const;
    void setMat4(const std::string& name, const Mat4& m) const;
    void setMat3(const std::string& name, const float* m3x3) const;

private:
    static std::string readFile(const std::string& path);
    static GLuint compileShader(GLenum type, const std::string& source, const std::string& path);
};

#endif // SHADER_H