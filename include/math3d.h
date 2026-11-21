#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>
#include <iostream>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 2D Vector
struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& b) const { return Vec2(x + b.x, y + b.y); }
    Vec2 operator-(const Vec2& b) const { return Vec2(x - b.x, y - b.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2& operator+=(const Vec2& b) { x += b.x; y += b.y; return *this; }
    Vec2& operator-=(const Vec2& b) { x -= b.x; y -= b.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float lengthSq() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSq()); }
    Vec2 normalized() const {
        float l = length();
        return (l > 1e-6f) ? Vec2(x / l, y / l) : Vec2(0.0f, 0.0f);
    }
};

// 3D Vector
struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& b) const { return Vec3(x + b.x, y + b.y, z + b.z); }
    Vec3 operator-(const Vec3& b) const { return Vec3(x - b.x, y - b.y, z - b.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator*(const Vec3& b) const { return Vec3(x * b.x, y * b.y, z * b.z); }

    Vec3& operator+=(const Vec3& b) { x += b.x; y += b.y; z += b.z; return *this; }
    Vec3& operator-=(const Vec3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec3 normalized() const {
        float len = length();
        if (len > 1e-7f) {
            return Vec3(x / len, y / len, z / len);
        }
        return Vec3(0.0f, 1.0f, 0.0f);
    }

    static float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a * (1.0f - t) + b * t;
    }
};

inline Vec3 operator*(float s, const Vec3& v) {
    return v * s;
}

// 4D Vector
struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4(const Vec3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    Vec3 xyz() const { return Vec3(x, y, z); }
    Vec4 operator+(const Vec4& b) const { return Vec4(x + b.x, y + b.y, z + b.z, w + b.w); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
};

// 4x4 Matrix (Column-Major)
struct Mat4 {
    float m[16];

    Mat4() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    const float* data() const { return m; }
    float* data() { return m; }

    static Mat4 identity() {
        return Mat4();
    }

    float at(int row, int col) const { return m[col * 4 + row]; }
    float& at(int row, int col) { return m[col * 4 + row]; }

    Mat4 operator*(const Mat4& b) const {
        Mat4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += at(r, k) * b.at(k, c);
                }
                res.at(r, c) = sum;
            }
        }
        return res;
    }

    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            at(0,0)*v.x + at(0,1)*v.y + at(0,2)*v.z + at(0,3)*v.w,
            at(1,0)*v.x + at(1,1)*v.y + at(1,2)*v.z + at(1,3)*v.w,
            at(2,0)*v.x + at(2,1)*v.y + at(2,2)*v.z + at(2,3)*v.w,
            at(3,0)*v.x + at(3,1)*v.y + at(3,2)*v.z + at(3,3)*v.w
        );
    }

    static Mat4 translate(float tx, float ty, float tz) {
        Mat4 res = identity();
        res.at(0, 3) = tx;
        res.at(1, 3) = ty;
        res.at(2, 3) = tz;
        return res;
    }

    static Mat4 translate(const Vec3& v) {
        return translate(v.x, v.y, v.z);
    }

    static Mat4 scale(float sx, float sy, float sz) {
        Mat4 res = identity();
        res.at(0, 0) = sx;
        res.at(1, 1) = sy;
        res.at(2, 2) = sz;
        return res;
    }

    static Mat4 scale(float s) {
        return scale(s, s, s);
    }

    static Mat4 rotateX(float angle_rad) {
        Mat4 res = identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        res.at(1, 1) = c;  res.at(1, 2) = -s;
        res.at(2, 1) = s;  res.at(2, 2) = c;
        return res;
    }

    static Mat4 rotateY(float angle_rad) {
        Mat4 res = identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        res.at(0, 0) = c;  res.at(0, 2) = s;
        res.at(2, 0) = -s; res.at(2, 2) = c;
        return res;
    }

    static Mat4 rotateZ(float angle_rad) {
        Mat4 res = identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        res.at(0, 0) = c;  res.at(0, 1) = -s;
        res.at(1, 0) = s;  res.at(1, 1) = c;
        return res;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = Vec3::cross(f, up).normalized();
        Vec3 u = Vec3::cross(s, f);

        Mat4 res = identity();
        res.at(0, 0) = s.x;  res.at(0, 1) = s.y;  res.at(0, 2) = s.z;  res.at(0, 3) = -Vec3::dot(s, eye);
        res.at(1, 0) = u.x;  res.at(1, 1) = u.y;  res.at(1, 2) = u.z;  res.at(1, 3) = -Vec3::dot(u, eye);
        res.at(2, 0) = -f.x; res.at(2, 1) = -f.y; res.at(2, 2) = -f.z; res.at(2, 3) =  Vec3::dot(f, eye);
        res.at(3, 0) = 0.0f; res.at(3, 1) = 0.0f; res.at(3, 2) = 0.0f; res.at(3, 3) = 1.0f;
        return res;
    }

    static Mat4 perspective(float fovy_rad, float aspect, float zNear, float zFar) {
        Mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;
        float tanHalf = std::tan(fovy_rad * 0.5f);
        res.at(0, 0) = 1.0f / (aspect * tanHalf);
        res.at(1, 1) = 1.0f / tanHalf;
        res.at(2, 2) = -(zFar + zNear) / (zFar - zNear);
        res.at(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        res.at(3, 2) = -1.0f;
        return res;
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float zNear = -1.0f, float zFar = 1.0f) {
        Mat4 res = identity();
        res.at(0, 0) = 2.0f / (right - left);
        res.at(1, 1) = 2.0f / (top - bottom);
        res.at(2, 2) = -2.0f / (zFar - zNear);
        res.at(0, 3) = -(right + left) / (right - left);
        res.at(1, 3) = -(top + bottom) / (top - bottom);
        res.at(2, 3) = -(zFar + zNear) / (zFar - zNear);
        return res;
    }

    Mat4 inverse() const {
        Mat4 inv;
        float invArr[16];
        const float* m_ = m;

        invArr[0] = m_[5]  * m_[10] * m_[15] - 
                    m_[5]  * m_[11] * m_[14] - 
                    m_[9]  * m_[6]  * m_[15] + 
                    m_[9]  * m_[7]  * m_[14] +
                    m_[13] * m_[6]  * m_[11] - 
                    m_[13] * m_[7]  * m_[10];

        invArr[4] = -m_[4]  * m_[10] * m_[15] + 
                     m_[4]  * m_[11] * m_[14] + 
                     m_[8]  * m_[6]  * m_[15] - 
                     m_[8]  * m_[7]  * m_[14] - 
                     m_[12] * m_[6]  * m_[11] + 
                     m_[12] * m_[7]  * m_[10];

        invArr[8] = m_[4]  * m_[9] * m_[15] - 
                    m_[4]  * m_[11] * m_[13] - 
                    m_[8]  * m_[5] * m_[15] + 
                    m_[8]  * m_[7] * m_[13] + 
                    m_[12] * m_[5] * m_[11] - 
                    m_[12] * m_[7] * m_[9];

        invArr[12] = -m_[4]  * m_[9] * m_[14] + 
                      m_[4]  * m_[10] * m_[13] +
                      m_[8]  * m_[5] * m_[14] - 
                      m_[8]  * m_[6] * m_[13] - 
                      m_[12] * m_[5] * m_[10] + 
                      m_[12] * m_[6] * m_[9];

        invArr[1] = -m_[1]  * m_[10] * m_[15] + 
                     m_[1]  * m_[11] * m_[14] + 
                     m_[9]  * m_[2] * m_[15] - 
                     m_[9]  * m_[3] * m_[14] - 
                     m_[13] * m_[2] * m_[11] + 
                     m_[13] * m_[3] * m_[10];

        invArr[5] = m_[0]  * m_[10] * m_[15] - 
                    m_[0]  * m_[11] * m_[14] - 
                    m_[8]  * m_[2] * m_[15] + 
                    m_[8]  * m_[3] * m_[14] + 
                    m_[12] * m_[2] * m_[11] - 
                    m_[12] * m_[3] * m_[10];

        invArr[9] = -m_[0]  * m_[9] * m_[15] + 
                     m_[0]  * m_[11] * m_[13] + 
                     m_[8]  * m_[1] * m_[15] - 
                     m_[8]  * m_[3] * m_[13] - 
                     m_[12] * m_[1] * m_[11] + 
                     m_[12] * m_[3] * m_[9];

        invArr[13] = m_[0]  * m_[9] * m_[14] - 
                     m_[0]  * m_[10] * m_[13] - 
                     m_[8]  * m_[1] * m_[14] + 
                     m_[8]  * m_[2] * m_[13] + 
                     m_[12] * m_[1] * m_[10] - 
                     m_[12] * m_[2] * m_[9];

        invArr[2] = m_[1]  * m_[6] * m_[15] - 
                    m_[1]  * m_[7] * m_[14] - 
                    m_[5]  * m_[2] * m_[15] + 
                    m_[5]  * m_[3] * m_[14] + 
                    m_[13] * m_[2] * m_[7] - 
                    m_[13] * m_[3] * m_[6];

        invArr[6] = -m_[0]  * m_[6] * m_[15] + 
                     m_[0]  * m_[7] * m_[14] + 
                     m_[4]  * m_[2] * m_[15] - 
                     m_[4]  * m_[3] * m_[14] - 
                     m_[12] * m_[2] * m_[7] + 
                     m_[12] * m_[3] * m_[6];

        invArr[10] = m_[0]  * m_[5] * m_[15] - 
                     m_[0]  * m_[7] * m_[13] - 
                     m_[4]  * m_[1] * m_[15] + 
                     m_[4]  * m_[3] * m_[13] + 
                     m_[12] * m_[1] * m_[7] - 
                     m_[12] * m_[3] * m_[5];

        invArr[14] = -m_[0]  * m_[5] * m_[14] + 
                      m_[0]  * m_[6] * m_[13] + 
                      m_[4]  * m_[1] * m_[14] - 
                      m_[4]  * m_[2] * m_[13] - 
                      m_[12] * m_[1] * m_[6] + 
                      m_[12] * m_[2] * m_[5];

        invArr[3] = -m_[1] * m_[6] * m_[11] + 
                     m_[1] * m_[7] * m_[10] + 
                     m_[5] * m_[2] * m_[11] - 
                     m_[5] * m_[3] * m_[10] - 
                     m_[9] * m_[2] * m_[7] + 
                     m_[9] * m_[3] * m_[6];

        invArr[7] = m_[0] * m_[6] * m_[11] - 
                    m_[0] * m_[7] * m_[10] - 
                    m_[4] * m_[2] * m_[11] + 
                    m_[4] * m_[3] * m_[10] + 
                    m_[8] * m_[2] * m_[7] - 
                    m_[8] * m_[3] * m_[6];

        invArr[11] = -m_[0] * m_[5] * m_[11] + 
                      m_[0] * m_[7] * m_[9] + 
                      m_[4] * m_[1] * m_[11] - 
                      m_[4] * m_[3] * m_[9] - 
                      m_[8] * m_[1] * m_[7] + 
                      m_[8] * m_[3] * m_[5];

        invArr[15] = m_[0] * m_[5] * m_[10] - 
                     m_[0] * m_[6] * m_[9] - 
                     m_[4] * m_[1] * m_[10] + 
                     m_[4] * m_[2] * m_[9] + 
                     m_[8] * m_[1] * m_[6] - 
                     m_[8] * m_[2] * m_[5];

        float det = m_[0] * invArr[0] + m_[1] * invArr[4] + m_[2] * invArr[8] + m_[3] * invArr[12];
        if (std::fabs(det) < 1e-7f) {
            return Mat4::identity();
        }

        float invDet = 1.0f / det;
        for (int i = 0; i < 16; i++) {
            inv.m[i] = invArr[i] * invDet;
        }
        return inv;
    }

    void getNormalMatrix(float* out3x3) const {
        float a00 = at(0,0), a01 = at(0,1), a02 = at(0,2);
        float a10 = at(1,0), a11 = at(1,1), a12 = at(1,2);
        float a20 = at(2,0), a21 = at(2,1), a22 = at(2,2);

        float det = a00 * (a11 * a22 - a12 * a21) -
                    a01 * (a10 * a22 - a12 * a20) +
                    a02 * (a10 * a21 - a11 * a20);

        if (std::fabs(det) < 1e-7f) {
            out3x3[0] = 1.0f; out3x3[3] = 0.0f; out3x3[6] = 0.0f;
            out3x3[1] = 0.0f; out3x3[4] = 1.0f; out3x3[7] = 0.0f;
            out3x3[2] = 0.0f; out3x3[5] = 0.0f; out3x3[8] = 1.0f;
            return;
        }

        float invDet = 1.0f / det;
        out3x3[0] =  (a11 * a22 - a12 * a21) * invDet;
        out3x3[1] = -(a01 * a22 - a02 * a21) * invDet;
        out3x3[2] =  (a01 * a12 - a02 * a11) * invDet;

        out3x3[3] = -(a10 * a22 - a12 * a20) * invDet;
        out3x3[4] =  (a00 * a22 - a02 * a20) * invDet;
        out3x3[5] = -(a00 * a12 - a02 * a10) * invDet;

        out3x3[6] =  (a10 * a21 - a11 * a20) * invDet;
        out3x3[7] = -(a00 * a21 - a01 * a20) * invDet;
        out3x3[8] =  (a00 * a11 - a01 * a10) * invDet;
    }
};

#endif // MATH3D_H