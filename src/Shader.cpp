#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Shader::Shader() : id(0) {}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) : id(0) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (id != 0) {
        glDeleteProgram(id);
    }
}

Shader::Shader(Shader&& other) noexcept : id(other.id) {
    other.id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (id != 0) {
            glDeleteProgram(id);
        }
        id = other.id;
        other.id = 0;
    }
    return *this;
}

std::string Shader::readFile(const std::string& path) {
    std::vector<std::string> searchPaths = {
        path,
        "shaders/" + path,
        "../" + path,
        "../../" + path,
        "build/Release/" + path
    };

    std::ifstream file;
    std::string foundPath = "";
    for (const auto& sp : searchPaths) {
        file.open(sp);
        if (file.is_open()) {
            foundPath = sp;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[Shader] Error: Could not open file " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compileShader(GLenum type, const std::string& source, const std::string& path) {
    GLuint shader = glCreateShader(type);
    const char* srcPtr = source.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(logLen > 0 ? logLen : 1024);
        glGetShaderInfoLog(shader, (GLsizei)infoLog.size(), nullptr, infoLog.data());
        std::cerr << "[Shader] Compilation error in " << path << ":\n" << infoLog.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertCode = readFile(vertexPath);
    std::string fragCode = readFile(fragmentPath);

    if (vertCode.empty() || fragCode.empty()) {
        return false;
    }

    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertCode, vertexPath);
    if (!vertShader) return false;

    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragCode, fragmentPath);
    if (!fragShader) {
        glDeleteShader(vertShader);
        return false;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(logLen > 0 ? logLen : 1024);
        glGetProgramInfoLog(program, (GLsizei)infoLog.size(), nullptr, infoLog.data());
        std::cerr << "[Shader] Linking error (" << vertexPath << ", " << fragmentPath << "):\n"
                  << infoLog.data() << std::endl;
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return false;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    if (id != 0) {
        glDeleteProgram(id);
    }
    id = program;
    return true;
}

void Shader::use() const {
    if (id != 0) {
        glUseProgram(id);
    }
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const Vec2& v) const {
    glUniform2f(glGetUniformLocation(id, name.c_str()), v.x, v.y);
}

void Shader::setVec3(const std::string& name, const Vec3& v) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z);
}

void Shader::setVec4(const std::string& name, const Vec4& v) const {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z, v.w);
}

void Shader::setMat4(const std::string& name, const Mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, m.data());
}

void Shader::setMat3(const std::string& name, const float* m3x3) const {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, m3x3);
}
